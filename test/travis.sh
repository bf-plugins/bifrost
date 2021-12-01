#!/bin/bash
# This file runs CPU-safe tests for travis-ci
./download_test_data.sh
export LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}
python -c "from bifrost import telemetry; telemetry.disable()"
coverage run --source=bifrost.ring,bifrost,bifrost.pipeline -m unittest discover
