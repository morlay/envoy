#!/bin/bash

# Configure environment for Envoy Filter Example build and test.

set -e

# This is the hash on https://github.com/envoyproxy/envoy-filter-example.git we pin to.
ENVOY_FILTER_EXAMPLE_GITSHA="c6c986cca7ad676cc1c33f2df7515cbbd2e02502"
ENVOY_FILTER_EXAMPLE_SRCDIR="${BUILD_DIR}/envoy-filter-example"

export ENVOY_FILTER_EXAMPLE_TESTS="//:echo2_integration_test //http-filter-example:http_filter_integration_test //:envoy_binary_test"

if [[ ! -d "${ENVOY_FILTER_EXAMPLE_SRCDIR}/.git" ]]; then
  rm -rf "${ENVOY_FILTER_EXAMPLE_SRCDIR}"
  git clone https://github.com/envoyproxy/envoy-filter-example.git "${ENVOY_FILTER_EXAMPLE_SRCDIR}"
fi

(cd "${ENVOY_FILTER_EXAMPLE_SRCDIR}" && git fetch origin && git checkout -f "${ENVOY_FILTER_EXAMPLE_GITSHA}")
sed -e "s|{ENVOY_SRCDIR}|${ENVOY_SRCDIR}|" "${ENVOY_SRCDIR}"/ci/WORKSPACE.filter.example > "${ENVOY_FILTER_EXAMPLE_SRCDIR}"/WORKSPACE

mkdir -p "${ENVOY_FILTER_EXAMPLE_SRCDIR}"/bazel
ln -sf "${ENVOY_SRCDIR}"/bazel/get_workspace_status "${ENVOY_FILTER_EXAMPLE_SRCDIR}"/bazel/
cp -f "${ENVOY_SRCDIR}"/.bazelrc "${ENVOY_FILTER_EXAMPLE_SRCDIR}"/
cp -f --remove-destination "${ENVOY_SRCDIR}"/.bazelversion "${ENVOY_FILTER_EXAMPLE_SRCDIR}"/
cp -f "$(bazel info workspace)"/*.bazelrc "${ENVOY_FILTER_EXAMPLE_SRCDIR}"/

FILTER_WORKSPACE_SET=1
