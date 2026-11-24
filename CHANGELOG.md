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
* `--save-pars-2d <par1,par2,...>`: save the profiled values of these parameters at each point of a 2D Prob scan to
  the scanner file, as histograms `hPar_<par>`.
* `Utils::makeTheoryVar(name, formula, pars)` overload, with title = name.
* `hflavPdg26` config tag.
* `scripts/lbd_condor_reqs.txt` (condor requirements for the LBD group).
* Python (`gc_core.mpl_tools`): `read2dpar`, `slice_at_y` and `mask_negative_r` (handle the r < 0 mirror in 2D
  scans), `read_gc_plugin_scan`, `read_par_from_dat`, and `text_logo` (generic multi-line logo).
* `gc_core` Python dependencies declared in `pyproject.toml`.

### Changed
* CL interval files carry the engine name: `plots/cl/clintervals_<engine>_<combiner>_<var>_<method>.py` (was
  `clintervals_<combiner>_...`). Scripts reading the old names need updating.
* `scripts/cam_condor_reqs.txt`: avoid black-hole nodes, request 2 GB, cap at 400 jobs.
* Python: `lhcb_logo` is built on `text_logo`; `pos` is now the top-left corner of the logo (was the baseline of
  "LHCb"), and the font follows `text.usetex`.
* Python: `corr_plot` draws vector cells (no rasterised EPS), scales fonts and annotations to the cells, leaves NaN
  cells empty, and takes an optional `scale`.
* Python: `df_corr_to_latex` writes a siunitx `tabular` directly and no longer returns the dataframe.
* Ruff configuration moved from `ruff.toml` to `pyproject.toml`.
* Toy directories and files carry the engine name, e.g. `root/scan1dPlugin_<engine>_<combiner>_<var>/` (also 2D
  plugin and coverage). Toys written before are read with `--toyFiles`; the merge prints the option when it finds
  them.
* Batch jobs source the LCG view of the submitting shell (`LCG_VIEW_DIR`) instead of the removed
  `scripts/setup_lxplus.sh`, and fail with exit code 1 if the setup fails.
* Batch job arguments are shell-quoted.
* The per-toy fit to the bkg-only toys (CLs) only runs with `--cls`, about a third less CPU. Merging with `--cls`
  needs toys made with `--cls`.
* Combiner scanner files only hold the CLs histograms that are computed: `hCLsFreq`, `hCLsExp` and `hCLsErr*` only
  for Plugin scans with `--cls`, `hCLs` only for Prob scans with `--cls` (no longer for Plugin scans). Loading such a
  file with `--cls 1` warns that `hCLs` is missing. Datasets scanner files are unchanged.
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
* `Combiner::Clone` dropped the fixed parameters.
* CL intervals of angles whose best fit is wrapped 2π outside the scan range (e.g. phiD just below 0).
* LaTeX macro names with digits in `RooSlimFitResult::SaveLatex` (`x12`, `y12`).
* Plugin scans with `--cls 1` used an empty `hCLs` (1-CL = 0 everywhere) for the CLs intervals and plot. The
  intervals now fall back to normalising `hCL` to its first bin (none if its p value is 0); the plot leaves out the
  CLs type 1 curve, with a warning.
* Segfault when loading a scanner file without `hCLs` with `--cls 1`.
* Python: `read_minimum` with parameters held constant; `read_gc_scan` crash on file close; `corr_plot` with
  read-only pandas >= 3.0 views.
