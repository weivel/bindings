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
