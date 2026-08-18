# Slugs

A 2D physics-based game written in C++20.

## Overview

This project is a work-in-progress (WIP) custom 2D physics game focused on low-level performance, custom collision handling, and interactive rigid body dynamics.

## Key Features

* **C++20 Implementation:** Modern C++ practices focusing on type safety, high performance, and clean architecture.
* **Custom Physics Engine:** Custom-built 2D physics engine handling integrations, velocities, and particle/rigid body collisions.
* **Modular Structure:** Clean separation between physics core, rendering pipeline, and entity logic.

## Technical Details

* **Language:** C++23
* **Focus Areas:** Systems programming, performance optimization, custom algorithms.

## Building and Running

### Prerequisites

* C++23 compatible compiler (GCC, Clang, or MSVC)
* CMake (v3.16+)

### Build Steps

```bash
git clone [https://github.com/Sebo1411/Slugs.git](https://github.com/Sebo1411/Slugs.git)
cd Slugs

1. Visual Studio (MSVC) configuration
cmake -B build_msvc -G "Visual Studio 17 2022"

# 2. Compiling and building (Release profile)
cmake --build build_msvc --config Release

2. GCC (Ninja / Make) configuration
cmake -B build_gcc -G "Ninja" -DCMAKE_BUILD_TYPE=Debug

# 2. Compiling and building (Debug profile)
cmake --build build_gcc