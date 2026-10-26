## What this is

How each language renders the weivel plugin model. A binding specification pins the SURFACE in one
language: the names, the shapes, and how each invariant of
[`weivel/model`](https://github.com/weivel/model)'s `INVARIANTS.md` appears in that language's
types. It exists so two implementations of one language are interchangeable, which a conformance
suite cannot deliver alone: a plugin compiles against types and names, not against behaviour.

**What it is not.** The model, which is language-neutral and lives in `weivel/model`. The behaviour,
which [`weivel/conformance`](https://github.com/weivel/conformance) pins as vectors. And the
reference documentation of an implementation, which each implementation publishes from its own
source.

## Layout

- `java-and-typescript.md`: the two languages that already have implementations, specified together
  because the second was checked against the first line by line.

Every language's specification lives here rather than beside its implementation, so the languages
stay comparable and a drift between two of them is visible in one place. The developer guides for
plugin, host and implementation authors will live here too, beside the specification each one is
the readable companion of.

## Changing a binding

A binding specification is normative. Change it first, then make the implementation match it; an
implementation that disagrees with its binding is the defect, not the document.

## The echo fixture

`fixtures/echo` is a neutral C library that exists only so an implementation can prove its native
loading end to end: it downloads the release, verifies it by SHA-256 and calls four functions that
succeed, fail on purpose, take an argument and return bytes that are not UTF-8. It declares itself
in `fixtures/echo/library.json`, which is a copy of the model's `echo.library.json` fixture.

Pushing a `v*` tag builds it for `linux-x64` and `win32-x64` through the shared library workflow and
publishes a release holding the declaration, the two binaries and a `SHA256SUMS` to pin. `v0.1.0` is
the first.
