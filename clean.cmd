CD alib
IF EXIST obj\\*.o DEL obj\\*.o
IF EXIST lib\\*.a DEL lib\\*.a
CD ..\\csvfix
IF EXIST obj\\*.o DEL obj\\*.o
IF EXIST bin\\*.exe DEL bin\\*.exe
CD ..
