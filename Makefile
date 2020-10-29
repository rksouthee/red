
CPPFLAGS = /nologo /EHsc /W4 /DNOMINMAX
LINK = link.exe
LDFLAGS = /NOLOGO

DEBUG = 1
PDB = 1

!IF $(DEBUG) == 1
CPPFLAGS = $(CPPFLAGS) /Zi /Od
LDFLAGS = $(LDFLAGS) /DEBUG
!ELSE
CPPFLAGS = $(CPPFLAGS) /O2
!IF $(PDB) == 1
CPPFLAGS = $(CPPFLAGS) /Zi
LDFLAGS = $(LDFLAGS) /DEBUG
!ENDIF
!ENDIF

SOURCES = main.cpp buffer.cpp gap_buffer.cpp
OBJECTS = $(SOURCES:.cpp=.obj)
PROGRAM = red

all: $(PROGRAM).exe

.PHONY: clean all

$(PROGRAM).exe: $(OBJECTS)
	$(LINK) $(LDFLAGS) /OUT:$@ $(OBJECTS)

clean:
	@if exist *.exe erase *.exe
	@if exist *.obj erase *.obj
	@if exist *.pdb erase *.pdb
	@if exist *.ilk erase *.ilk
