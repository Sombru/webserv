#!/bin/bash

ab -n 10 -c 5 http://localhost:8080/

# -n 10: total number of requests

# -c 5: concurrency (clients at the same time)