# Introduction

[![run-unit-tests](https://github.com/kkimber/CBUSPicoLib/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/kkimber/CBUSPicoLib/actions/workflows/unit_tests.yml) [![CodeQL](https://github.com/kkimber/CBUSPicoLib/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/kkimber/CBUSPicoLib/actions/workflows/github-code-scanning/codeql) [![CodeFactor](https://www.codefactor.io/repository/github/kkimber/cbuspicolib/badge)](https://www.codefactor.io/repository/github/kkimber/cbuspicolib)

This repository provides source code CBUS functionality on a Raspberry Pico.

The code provides:

  * Basic CBUS support for a SLiM or FLiM module over CAN, using a soft PIO based CAN controller
  * Support for the GridConnect protocol over WiFi on a Pico-W for integration with FCU orJMRI
  * Support for extended CBUS protocols, such as long messages

The code is not meant to be used in isolation, and requires other code and supporting files to be useful.

For a simple example using this code, see the repository : https://github.com/kkimber/CBUSPico
