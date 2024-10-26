# Introduction

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC_BY--NC--SA_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/) [![run-unit-tests](https://github.com/kkimber/CBUSPicoLib/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/kkimber/CBUSPicoLib/actions/workflows/unit_tests.yml) [![CodeQL](https://github.com/kkimber/CBUSPicoLib/actions/workflows/codeql.yml/badge.svg)](https://github.com/kkimber/CBUSPicoLib/actions/workflows/codeql.yml) [![CodeFactor](https://www.codefactor.io/repository/github/kkimber/cbuspicolib/badge)](https://www.codefactor.io/repository/github/kkimber/cbuspicolib)

The Unit Test framework uses "mocklib" code derived from the project [SmartFilamentSensor](https://github.com/slavaz/SmartFilamentSensor) which is licensed GPL v3:

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This repository provides source code CBUS functionality on a Raspberry Pico.

The code provides:

  * Basic CBUS support for a SLiM or FLiM module over CAN, using a soft PIO based CAN controller
  * Support for the GridConnect protocol over WiFi on a Pico-W for integration with FCU or JMRI
  * Support for extended CBUS protocols, such as long messages

The code is not meant to be used in isolation, and requires other code and supporting files to be useful.

For a simple example using this code, see the repository : https://github.com/kkimber/CBUSPico
