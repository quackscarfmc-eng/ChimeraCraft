# SPDX-FileCopyrightText: 2023 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

set(INC
  ../include
  ../../makesrna
  ../../../../intern/mantaflow/extern
  # RNA_prototypes.hh
  ${CMAKE_BINARY_DIR}/source/blender/makesrna
)

set(INC_SYS
)

set(SRC
  dynamicpaint_ops.cc
  particle_boids.cc
  particle_edit.cc
  particle_edit_undo.cc
  particle_object.cc
  physics_fluid.cc
  physics_ops.cc
  physics_pointcache.cc
  rigidbody_constraint.cc
  rigidbody_object.cc
  rigidbody_world.cc

  particle_edit_utildefines.h
  physics_intern.hh
)

# NOTE: the unfinished in-process ZOZO / PPF contact solver integration
# ("Step 1.2 Incomplete") was reverted. It was gated behind an OFF-by-default
# WITH_PPF_CONTACT_SOLVER option, built a Rust staticlib with a Cargo custom
# command, and exposed only a lifecycle smoke test (begin/end) over a C ABI
# that accepted Rust-owned scene pointers with no constructors. Its
# CONFIGURE_DEPENDS glob also named "${PPF_CTS_ROOT}/build.rs", a file that
# does not exist (the real one is crates/ppf-cts-solver/build.rs). Nothing in
# this directory now builds, links or defines WITH_PPF_CONTACT_SOLVER. See
# README.md, "ZOZO / PPF contact solver".

set(LIB
  PRIVATE bf::blenkernel
  PRIVATE bf::blenlib
  PRIVATE bf::blentranslation
  PRIVATE bf::depsgraph
  PRIVATE bf::dna
  PRIVATE bf::gpu
  PRIVATE bf::intern::clog
  PRIVATE bf::intern::guardedalloc
  PRIVATE bf::windowmanager
)

if(WITH_MOD_FLUID)
  list(APPEND LIB
    bf_intern_mantaflow
  )
  add_definitions(-DWITH_FLUID)
endif()

if(WITH_BULLET)
  list(APPEND INC
    ../../../../intern/rigidbody
  )
  add_definitions(-DWITH_BULLET)
endif()


blender_add_lib(bf_editor_physics "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

# RNA_prototypes.hh
add_dependencies(bf_editor_physics bf_rna)
