#!/bin/sh

set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

"$REPO_DIR/autotest/atrunner/build/dqatrun" \
  -c "$REPO_DIR/compiler/build/dq-comp" \
  -r "$REPO_DIR/autotest/tests"

