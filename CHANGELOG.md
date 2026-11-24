# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.1.0/)
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## 1.4.0 [Unreleased]

### Added
* `--fix-from-parfile <par1,par2,...>`: fix parameters to their values in the parameter file given with
  `--fix-parfile` (or, if not given, `--parfile`). Like `--fix`, given once it applies to all combinations.
* `GammaComboEngine::setBatchArgs()`: command line rerun by the batch jobs, for executables that strip their own
  options before building the engine.
* `GammaComboEngine::getCombinersIds()` and `GammaComboEngine::getPdfs()`.
* `Utils::makeTheoryVar(name, formula, pars)` overload, with title = name. Repeated spaces are removed from the
  formula.
* Python package `gc_core` (`pip install -e .`), with the former `scripts/gc_mpl_tools.py` (`gc_core.mpl_tools`) and
  `scripts/pyfitres.py` (`gc_core.pyfitres`), and `gc_core.utils.load_gc_core_lib()`. New in `gc_core.mpl_tools`:
  `text_logo`, `read2dpar`, `slice_at_y`, `mask_negative_r`, `read_gc_plugin_scan` and `read_par_from_dat`; extra
  lines/markers in legends.
* CI: `test-python` job and `.python_venv` template (GitLab).

### Changed
* Setting constant a parameter that is not in the workspace is a warning (was an error).
* `--fix` given a single time applies to all combinations.
* Batch job arguments are shell-quoted.
* Python 3.10 or newer is required. The ruff configuration moved from `ruff.toml` to `pyproject.toml`.
* Python: `lhcb_logo` is built on `text_logo`; `pos` is now the top-left corner of the logo (was the baseline of
  "LHCb"). `corr_plot` draws vector cells and takes an optional `scale`. `df_corr_to_latex` writes a siunitx
  `tabular` directly and no longer returns the dataframe.
* Removed the deprecated `scripts/gc_mpl.py`.
* Removed stateless classes `ColorBuilder`, `FitResultDump` and `TGraphTools`
* Moved `float` -> `double` for all variables except for the ones stored in
  `TTree`s and the ones that are `float` in ROOT (`Minuit` internally uses
  `double`).

### Fixed
* Const correctness of (most) class methods.
* Delete copy constructors and copy assignment operators for resource-managing
  classes.
* Headers are now self-sufficient.
* Crash when an executable is run without arguments.
* `Combiner::Clone` dropped the fixed parameters.
* CL intervals of angles whose best fit is wrapped 2π outside the scan range (e.g. phiD just below 0).
* Batch job output names were mangled when a combiner name contains "sub".
* Python: `read_minimum` with parameters held constant; `read_gc_scan` crash on file close; `corr_plot` with
  read-only pandas >= 3.0 views.
