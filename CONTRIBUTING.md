# Coding style
The project enforces a consistent coding style through a number of
[pre-commit](https://pre-commit.com/#install) hooks, including
[`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) and
[ruff](https://docs.astral.sh/ruff/).
The hooks to conform the code to the style before any commit should be installed
by typing `pre-commit install` once every time the repository is cloned
(requires a working installation of
[pre-commit](https://pre-commit.com/#install)).

You can run all the `pre-commit` hooks before committing the code
via `pre-commit run --all` or `pre-commit run --files <file paths>` to avoid
having to stage the files twice.
