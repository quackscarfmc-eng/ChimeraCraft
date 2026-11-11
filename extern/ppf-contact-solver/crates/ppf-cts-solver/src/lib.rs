// File: lib.rs
// Code: Claude Code and Codex
// Review: Ryoichi Ando (ryoichi.ando@zozo.com)
// License: Apache v2.0
//
// This crate is a standalone solver *binary*; `src/main.rs` is its real module
// root and declares every module the solver uses.
//
// The unfinished in-process Blender integration ("Step 1.2 Incomplete") made
// this file `include!("main.rs")` and declared `crate-type = ["staticlib"]`
// so Blender could link a `libppf_cts_solver.a`. That textually embedded the
// binary's `fn main()`, `env!("PPF_BACKEND")` and the `extern "C" set_log_path`
// declaration into a library, coupling `cargo build --lib` to build.rs and to
// whatever the binary happened to contain (issue-report.html, Z5). The C ABI
// it exported was unusable anyway: it accepted opaque pointers to Rust-owned
// DataSet/ParamSet records with no constructors (Z3), and Blender only ever
// called begin/end (Z2).
//
// That integration has been reverted, so the library target is intentionally
// empty. It is kept (as an rlib only) so `cargo build --lib` and `cargo test
// --lib` remain well-defined, but it exports no C ABI and no staticlib is
// produced. A future integration must define a real C ABI with constructors
// for scene records instead of reviving `include!("main.rs")`.
