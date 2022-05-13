# Simple automation script to run ClangBuildAnalyzer over the pioneer codebase
# and generate reports.
# Requires a clean build with the -DUSE_TIME_TRACE=1 cmake option passed to
# ./bootstrap, and should be run in the top-level directory of the pioneer repo

ANALYZER="$1"
if [ -z "$ANALYZER" ]; then
	ANALYZER="ClangBuildAnalyzer"
fi

$ANALYZER --all build/ build/build-pioneer-trace.bin 2&>/dev/null
$ANALYZER --analyze build/build-pioneer-trace.bin > build/build-pioneer-trace-$(date +%F-%T).txt
