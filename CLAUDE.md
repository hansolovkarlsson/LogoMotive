# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Start here

`scratch/daily-standup.md` — written at the end of the previous working day to
be read at the start of the next: where the tree was left, what went in, and
what is outstanding. `scratch/` is gitignored and is not part of this
repository, so the file is absent on a fresh clone and on any day that was not
closed out. When it is absent, `git log` and the documents named below are the
way in.

## What this is

LogoMotive — a Logo interpreter with turtle graphics, in C with GTK4: a
bytecode compiler and VM underneath, with concurrent multi-turtle agents and
sprites on top of the classic language. Runs natively on macOS, Linux and
Windows.

## Commands

`make`, `make run`, `make test`, `make clean`. The suite is split into
per-stage targets — `test-lexer`, `test-parser`, `test-eval`, `test-vm`,
`test-bytecode`, `test-agent`, `test-shadow-diff` — useful for narrowing a
failure.

## The records

`docs/ROADMAP.md` and `docs/CHANGELOG.md`. The reference documents
(`LANGUAGE.md`, `COMMAND_REFERENCE.md`, `BYTECODE_REFERENCE.md`, the design
notes and the tutorials) are the rest of `docs/`.

**This repository has no journal and no postmortem.** Do not create either
without being asked; the roadmap and the changelog are where a day's work goes.
