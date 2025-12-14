set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd "$SCRIPT_DIR"
  protoc -I. --kts_out=. \
  --plugin=protoc-gen-kts=../build/protoc-gen-kts \
  *.proto
popd