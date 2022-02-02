#!/bin/bash
echo "this is ./Scripts/package_ext_libs_finish.sh"
set -e

export MVK_EXT_LIB_DST_PATH="${PROJECT_DIR}/External/build/"

# Assign symlink to Latest
ln -sfn "${CONFIGURATION}" "${MVK_EXT_LIB_DST_PATH}/Latest"

echo "Clean MoltenVK to ensure the next MoltenVK build will use the latest external library versions."
pwd
# make --quiet clean
echo "./Scripts/package_ext_libs_finish.sh done"

