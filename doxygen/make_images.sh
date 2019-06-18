#!/bin/bash
mkdir -p images
for dot_file in graphviz/*.dot; do dot_file=$(basename $dot_file); dot -Tpng graphviz/${dot_file} -o images/${dot_file%.dot}.png; done
