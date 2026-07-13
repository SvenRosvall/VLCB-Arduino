# GitHub Workflows Quick Tutorial

This project uses three GitHub Actions workflows. Together they check project code, Arduino examples, and generated documentation.

## Why these workflows exist

They provide a common environment for all developers.

- Catch problems early on clean test machines
- Give the same checks to every contributor
- Keep CI concerns split by responsibility

## Where to find them

- `.github/workflows/ci-host-tests.yml`
- `.github/workflows/ci-arduino-build.yml`
- `.github/workflows/ci-docs.yml`

## At a glance

| Workflow | Purpose                                                                                                                         | Runs when |
| --- |---------------------------------------------------------------------------------------------------------------------------------| --- |
| `Host Tests` | Build and run host test executable (`testAll`) with CMake                                                                       | Someone opens a PR, pushes code, or runs it manually |
| `Arduino Firmware Build` | Compile all sketches in `examples/` for Arduino Uno                                                                             | A PR or push changes Arduino-related files, or someone runs it manually |
| `Docs` | Generate Doxygen HTML docs, build all markdown files via Jekyll, and deploy to [Github Pages for this library](https://svenrosvall.github.io/VLCB-Arduino). | A PR or push changes docs/source/config files, or someone runs it manually |

## Learn more about GitHub Actions

- Quickstart: https://docs.github.com/actions/quickstart
- Understand workflows: https://docs.github.com/actions/using-workflows
- Events and triggers: https://docs.github.com/actions/using-workflows/events-that-trigger-workflows

## 1) Host Tests workflow

File: `.github/workflows/ci-host-tests.yml`

Main steps:

1. Check out the repository
2. Configure CMake (`cmake -S . -B build`)
3. Build `testAll`
4. Run `./build/testAll`

This workflow makes sure:

- Core project code and tests still pass
- CMake setup is still valid

When this runs:

- On every PR and push
- Manual run from the Actions tab

## 2) Arduino Firmware Build workflow

File: `.github/workflows/ci-arduino-build.yml`

Main steps:

1. Check out the repository
2. Install Arduino CLI
3. Install `arduino:avr` core
4. Loop over all `examples/*` directories and compile detected `.ino` sketches for `arduino:avr:uno`

This workflow makes sure:

- All example sketches still compile
- The library still works with the examples on Arduino Uno

When this runs automatically:

- If a PR or push changes one of these paths:
  - `examples/**`
  - `src/**`
  - `library.properties`
  - `.github/workflows/ci-arduino-build.yml`

Manual runs are always possible via `workflow_dispatch`.

## 3) Docs workflow

File: `.github/workflows/docs.yml`

Main steps:

1. Check out the repository
2. Install `doxygen` and `graphviz`
3. Generate docs with:
   - `Doxygen.Sketch.conf`
   - `Doxygen.Library.conf`
4. Upload artifacts:
   - `html.sketch`
   - `html.library`

This workflow makes sure:

- Doxygen settings are valid
- Documentation can still be generated successfully

When this runs automatically:

- If a PR or push changes one of these paths:
  - `docs/**`
  - `src/**`
  - `Doxygen.*.conf`
  - `.github/workflows/docs.yml`

## How to run a workflow manually

1. Open the repository on GitHub
2. Go to **Actions**
3. Select workflow (`Host Tests`, `Arduino Firmware Build`, or `Docs`)
4. Click **Run workflow**
5. Choose branch and run

## How to inspect outputs

- Open a workflow run in the Actions tab
- Expand the failed step to see logs
- For `Docs`, download artifacts (`html-sketch`, `html-library`) from the run summary

## Before you open a PR

Before opening a PR, do a quick check:

- Make sure host tests pass
- If you changed library or example code, make sure Arduino examples still compile
- If you changed source files, docs files, or documentation settings, make sure docs still build

This keeps the review focused on your changes, not avoidable CI failures.
