# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.1.0/)
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## 1.4.0 [Unreleased]

### Added
* `--batchsetup <script>`: script sourced by the batch jobs to set up their environment.
* `GammaComboEngine::setBatchArgs()`: command line rerun by the batch jobs, for executables that strip their own
  options before building the engine.

### Changed
* Toy directories and files carry the engine name, e.g. `root/scan1dPlugin_<engine>_<combiner>_<var>/` (also 2D
  plugin and coverage). Toys written before are read with `--toyFiles`; the merge prints the option when it finds
  them.
* Batch jobs source the LCG view of the submitting shell (`LCG_VIEW_DIR`) instead of the removed
  `scripts/setup_lxplus.sh`, and fail with exit code 1 if the setup fails.
* Batch job arguments are shell-quoted.
* The per-toy fit to the bkg-only toys (CLs) only runs with `--cls`, about a third less CPU. Merging with `--cls`
  needs toys made with `--cls`.
* Removed stateless classes `ColorBuilder`, `FitResultDump` and `TGraphTools`
* Moved `float` -> `double` for all variables except for the ones stored in
  `TTree`s and the ones that are `float` in ROOT (`Minuit` internally uses
  `double`).

### Fixed
* Const correctness of (most) class methods.
* Delete copy constructors and copy assignment operators for resource-managing
  classes.
* Headers are now self-sufficient.
* Segfault in the datasets plugin scan (a scanner without a combiner).
* Batch job output names were mangled when a combiner name contains "sub".
