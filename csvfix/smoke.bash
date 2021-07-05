#!bash
echo "Running..."
CSVED=../bin/csvfix.exe
export CSVED
DIFF=/bin/diff
DIFFOUT=tests/tmp/_diffout
cd tests
if bash tests/smoke.test > tmp/smoke.test
then
	if $DIFF correct/smoke.test tmp/smoke.test
	then
		swine msg --caption 'CSVFix Smoke Test' --text 'PASSED  ' --icon info	
		exit 0
	else
		swine msg --caption 'CSVFix Smoke Test' --text 'FAILED' --icon stop	
		exit 1
	fi
else
	echo "Command execution problem"
	read x
fi
