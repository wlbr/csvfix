ifndef COMSPEC
	CLEAN =	(cd alib; $(MAKE) clean) && (cd csvfix; $(MAKE) clean)
else
	CLEAN = cmd.exe /c "clean.cmd"
endif

default:
	@echo "use 'make win', 'make lin' or 'make mac'"

win:
	cmd.exe /c "mkdirs.cmd"
	cd alib && $(MAKE) win
	cd csvfix && $(MAKE) win
	cd csvfix/bin; gzip csvfix.exe -c > csvfix.win64.gz

lin:
	mkdir -p alib/obj alib/lib csvfix/obj csvfix/bin
	cd alib; $(MAKE) lin
	cd csvfix; $(MAKE) lin
	cd csvfix/bin; gzip csvfix -c > csvfix.linux-amd64.gz

# build for mac on Mountain Lion
# see http://groups.google.com/group/csvfix/browse_thread/thread/33ec3e5f157c16dd
mac:
	mkdir -p alib/obj alib/lib csvfix/obj csvfix/bin
	cd alib; $(MAKE) lin CCTYPE=clang
	cd csvfix; $(MAKE) lin CCTYPE=clang
	cd csvfix/bin; gzip csvfix -c > csvfix.osx.gz


clean:
	$(CLEAN)
