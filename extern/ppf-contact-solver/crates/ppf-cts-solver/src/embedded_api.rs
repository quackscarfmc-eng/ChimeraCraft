//! Initial in-process API boundary for the Rust CPU solver.
//!
//! The scene records remain Rust-owned in this first increment. The opaque
//! pointers accepted by `initialize` are an internal bridge contract, not a
//! stable C representation of `DataSet` or `ParamSet`.

use std::ffi::{c_char, c_void};
use std::panic::{catch_unwind, AssertUnwindSafe};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Mutex, MutexGuard};

use crate::data::{DataSet, ParamSet, StepResult};

const API_VERSION: u32 = 1;

static API_CALL: Mutex<()> = Mutex::new(());
static ACTIVE: AtomicBool = AtomicBool::new(false);
static INITIALIZED: AtomicBool = AtomicBool::new(false);
static FAILED: AtomicBool = AtomicBool::new(false);
static BROKEN: AtomicBool = AtomicBool::new(false);
static LAST_ERROR: Mutex<Option<String>> = Mutex::new(None);

fn call_lock() -> MutexGuard<'static, ()> {
    API_CALL.lock().unwrap_or_else(|error| error.into_inner())
}

fn error_lock() -> MutexGuard<'static, Option<String>> {
    LAST_ERROR.lock().unwrap_or_else(|error| error.into_inner())
}

pub(crate) fn in_process_mode() -> bool {
    ACTIVE.load(Ordering::Acquire)
}

pub(crate) fn record_error(message: impl Into<String>) {
    let mut error = error_lock();
    if error.is_none() {
        *error = Some(message.into());
    }
    FAILED.store(true, Ordering::Release);
}

fn clear_error() {
    *error_lock() = None;
    FAILED.store(false, Ordering::Release);
}

fn active() -> bool {
    if ACTIVE.load(Ordering::Acquire) {
        true
    } else {
        record_error("PPF embedded solver is not active");
        false
    }
}

fn ready_to_step(operation: &str) -> bool {
    if !active() || FAILED.load(Ordering::Acquire) {
        return false;
    }
    if !INITIALIZED.load(Ordering::Acquire) {
        record_error(format!(
            "PPF embedded solver {operation} called before initialization"
        ));
        return false;
    }
    true
}

fn capture_panic(payload: Box<dyn std::any::Any + Send>) {
    let message = payload
        .downcast_ref::<String>()
        .map(String::as_str)
        .or_else(|| payload.downcast_ref::<&'static str>().copied())
        .unwrap_or("solver failed with a non-string panic");
    record_error(format!("PPF embedded solver: {message}"));
}

fn copy_error(buffer: *mut c_char, capacity: usize) -> usize {
    let error = error_lock();
    let Some(message) = error.as_deref() else {
        return 0;
    };
    let bytes = message.as_bytes();
    let required = bytes.len() + 1;
    if !buffer.is_null() && capacity > 0 {
        let copied = bytes.len().min(capacity - 1);
        // SAFETY: the caller provides `capacity` writable bytes. The copy is
        // bounded to leave room for the terminating NUL.
        unsafe {
            std::ptr::copy_nonoverlapping(bytes.as_ptr(), buffer.cast(), copied);
            *buffer.add(copied) = 0;
        }
    }
    required
}

/// Version of the embedded C ABI.
#[no_mangle]
pub extern "C" fn ppf_cts_embedded_api_version() -> u32 {
    API_VERSION
}

/// Start the process-wide CPU solver runtime.
///
/// The GPU backends are deliberately unavailable through this API until their
/// native backend failure paths can return without terminating the host.
#[no_mangle]
pub extern "C" fn ppf_cts_embedded_begin() -> bool {
    let _call = call_lock();
    if ACTIVE.load(Ordering::Acquire) {
        record_error("PPF embedded solver is already active");
        return false;
    }
    if BROKEN.load(Ordering::Acquire) {
        record_error("PPF embedded solver cannot restart after a cleanup panic");
        return false;
    }
    clear_error();
    #[cfg(not(feature = "cpu"))]
    {
        record_error("PPF embedded solver currently requires a build with the `cpu` feature");
        return false;
    }
    #[cfg(feature = "cpu")]
    {
        if let Err(payload) = catch_unwind(AssertUnwindSafe(crate::driver::reset)) {
            BROKEN.store(true, Ordering::Release);
            capture_panic(payload);
            return false;
        }
        INITIALIZED.store(false, Ordering::Release);
        ACTIVE.store(true, Ordering::Release);
        true
    }
}

/// Initialize the solver from Rust-owned `DataSet` and `ParamSet` records.
///
/// The pointers must refer to live records constructed by this solver build.
/// This first ABI slice intentionally does not define their C memory layout.
///
/// # Safety
/// `data` and `param` must point to valid instances of the solver's internal
/// `DataSet` and `ParamSet` types for the duration of this call.
#[no_mangle]
pub unsafe extern "C" fn ppf_cts_embedded_initialize(
    data: *const c_void,
    param: *const c_void,
) -> bool {
    let _call = call_lock();
    if !active() || FAILED.load(Ordering::Acquire) {
        return false;
    }
    if INITIALIZED.load(Ordering::Acquire) {
        record_error("PPF embedded solver is already initialized");
        return false;
    }
    if data.is_null() || param.is_null() {
        record_error("PPF embedded solver initialize received a null scene record");
        return false;
    }
    let initialized = catch_unwind(AssertUnwindSafe(|| {
        // SAFETY: forwarded from this function's caller contract.
        crate::driver::initialize(data.cast::<DataSet>(), param.cast::<ParamSet>())
    }));
    match initialized {
        Ok(true) => {
            INITIALIZED.store(true, Ordering::Release);
            true
        }
        Ok(false) => {
            let detail = crate::driver::fatal_detail();
            let message = if detail.is_null() {
                "solver initialization failed without a diagnostic".to_string()
            } else {
                // SAFETY: `fatal_detail` returns a process-lived NUL-terminated
                // string owned by the driver.
                unsafe { std::ffi::CStr::from_ptr(detail) }
                    .to_string_lossy()
                    .into_owned()
            };
            record_error(message);
            false
        }
        Err(payload) => {
            capture_panic(payload);
            false
        }
    }
}

/// Advance one solver step.
///
/// Returns false on an invalid lifecycle state or driver fatal error. The
/// convergence flags in `result` describe the outcome of a completed step.
/// After a driver failure, further calls are refused until
/// `ppf_cts_embedded_end`.
///
/// # Safety
/// `result` must point to writable storage for one solver `StepResult`.
#[no_mangle]
pub unsafe extern "C" fn ppf_cts_embedded_step(result: *mut c_void) -> bool {
    let _call = call_lock();
    if !ready_to_step("step") {
        return false;
    }
    if result.is_null() {
        record_error("PPF embedded solver step received a null result record");
        return false;
    }
    match catch_unwind(AssertUnwindSafe(|| {
        // SAFETY: forwarded from this function's caller contract.
        crate::driver::advance(result.cast::<StepResult>());
    })) {
        Ok(()) => true,
        Err(payload) => {
            capture_panic(payload);
            false
        }
    }
}

/// Read committed positions and solver state back into the Rust-owned scene.
///
/// # Safety
/// The `DataSet` passed to `ppf_cts_embedded_initialize` must still be live.
#[no_mangle]
pub extern "C" fn ppf_cts_embedded_fetch() -> bool {
    let _call = call_lock();
    if !ready_to_step("fetch") {
        return false;
    }
    match catch_unwind(AssertUnwindSafe(|| crate::driver::fetch())) {
        Ok(()) => true,
        Err(payload) => {
            capture_panic(payload);
            false
        }
    }
}

/// Copy the most recent error into `buffer`.
///
/// Returns the required buffer size including the terminating NUL. Passing a
/// null buffer or zero capacity is a size query. Returns zero when there is no
/// recorded error.
///
/// # Safety
/// If `buffer` is non-null and `capacity` is non-zero, it must address at least
/// `capacity` writable bytes.
#[no_mangle]
pub unsafe extern "C" fn ppf_cts_embedded_last_error(
    buffer: *mut c_char,
    capacity: usize,
) -> usize {
    copy_error(buffer, capacity)
}

/// End the runtime and release process-global solver state.
#[no_mangle]
pub extern "C" fn ppf_cts_embedded_end() -> bool {
    let _call = call_lock();
    if !ACTIVE.load(Ordering::Acquire) {
        record_error("PPF embedded solver is not active");
        return false;
    }
    ACTIVE.store(false, Ordering::Release);
    INITIALIZED.store(false, Ordering::Release);
    match catch_unwind(AssertUnwindSafe(crate::driver::reset)) {
        Ok(()) => {
            clear_error();
            true
        }
        Err(payload) => {
            BROKEN.store(true, Ordering::Release);
            capture_panic(payload);
            false
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    static TEST_LOCK: Mutex<()> = Mutex::new(());

    #[test]
    fn error_boundary_catches_driver_fatal_and_requires_shutdown() {
        let _test = TEST_LOCK.lock().unwrap_or_else(|error| error.into_inner());
        assert_eq!(ppf_cts_embedded_api_version(), API_VERSION);
        assert!(ppf_cts_embedded_begin());
        INITIALIZED.store(true, Ordering::Release);

        let mut result = std::mem::MaybeUninit::<StepResult>::uninit();
        // This intentionally invokes the driver's pre-initialize fatal path.
        // The FFI boundary must return an error to its caller, not exit Blender.
        assert!(!unsafe { ppf_cts_embedded_step(result.as_mut_ptr().cast()) });

        let required = unsafe { ppf_cts_embedded_last_error(std::ptr::null_mut(), 0) };
        assert!(required > 1);
        let mut message = vec![0u8; required];
        let copied =
            unsafe { ppf_cts_embedded_last_error(message.as_mut_ptr().cast(), message.len()) };
        assert_eq!(copied, required);
        let message = std::ffi::CStr::from_bytes_until_nul(&message)
            .unwrap()
            .to_string_lossy();
        assert!(message.contains("advance() was called before initialize()"));

        let mut later_result = std::mem::MaybeUninit::<StepResult>::uninit();
        assert!(!unsafe { ppf_cts_embedded_step(later_result.as_mut_ptr().cast()) });
        assert!(ppf_cts_embedded_end());
        assert_eq!(
            unsafe { ppf_cts_embedded_last_error(std::ptr::null_mut(), 0) },
            0
        );
    }

    #[test]
    fn null_scene_records_are_reported_without_entering_the_driver() {
        let _test = TEST_LOCK.lock().unwrap_or_else(|error| error.into_inner());
        assert!(ppf_cts_embedded_begin());
        assert!(!unsafe { ppf_cts_embedded_initialize(std::ptr::null(), std::ptr::null()) });
        let required = unsafe { ppf_cts_embedded_last_error(std::ptr::null_mut(), 0) };
        assert!(required > 1);
        assert!(ppf_cts_embedded_end());
    }
}
