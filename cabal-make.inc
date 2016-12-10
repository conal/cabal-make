# -*- Makefile -*-

########################################################################
#
# cabal-make -- an assistant for Haskell projects
#
# Version 0.6
#
# See http://haskell.org/haskellwiki/cabal-make
# 
# Copyright 2007 by Conal Elliott; BSD3 license.
# 
########################################################################

# Default target
# all:

install:

#### Default settings.  To override, define before including this file.

build-args ?= 

# Pass to "./setup configure"
configure-args ?= \
        $(extra-configure-args) \
	$(configure-dirs) \
	$(haddock-config)

show-configure-args:
	@echo $(configure-args)

haddock-config ?= $(source-args) $(comments-args)

# No longer necessary, since cabal's haddock command does what I want.
source-args ?= 

# Where to place the comments.
comments ?= http://haskell.org/haskellwiki/Talk:$(package)/Library

# Generate links to user comment pages
comments-args ?= \
  --haddock-option=--comments-base=$(comments) \
  --haddock-option=--comments-module=$(comments)/%{MODULE/.//}

# Where the haskell sources are to be found.  Note trailing "/".  If your
# sources are in your main project directory, use "top-src-dir=".
top-src-dir ?= src/

# Version with dots in place of slashes
top-src-dot = $(subst /,.,$(top-src-dir))

# Version with hyphens in place of slashes
top-src-dash = $(subst /,-,$(top-src-dir))

# Location of the hscolour css file
hscolour-css ?= hscolour.css

# For test-get-build
tmp-dir ?= /tmp

# Find command.  If you're on Windows, you may have a dos find earlier
# on your PATH than the MSYS or Cygwin find.
find ?= /usr/bin/find

# The user name above is for the make target "webdoc", which uses scp to copy
# your documentation to the server
user ?= UnknownUser

# Package name.   Extract from cabal file.
package ?= $(strip $(shell cat *.cabal | egrep -i 'name:' | sed -e 's/[Nn]ame://'))
show-package:
	@echo $(package)

# Note that the /I (case-insensitive) option doesn't work on macports.
# Otherwise, sed -e 's/name://i'.

# Extract modules from $(package).cabal.
modules = $(strip $(shell egrep '^[[:space:]]+[A-Z][0-9A-Za-z.]+[^.:][ \t]*$$' *.cabal))
show-modules:
	@echo $(modules)


# For seeing generated docs and setting up comment watch.
BROWSER ?= open

HsColour ?= HsColour


#### Rules

all:  configure build doc install

# I'm never sure which one of the following to use
# config-file = .setup-config
config-file = dist/setup-config

# configure: $(config-file)

# show-config:
# 	ls -l $(config-file)

# alias
# config: configure

# build: $(config-file)
# 	./setup build $(build-args)

build:
	cabal build

# On my (WinXP) system, cabal's install creates documentation directories
# that are not writable by me.

# install-dir ?= c:/haskell/packages
# docdir ?= $(install-dir)/doc/$(package)-$(version)

# Solution: stop using Windows

# install-was: build
# 	./setup install

install:
	cabal install $(install-flags)

#	mkdir -p $(docdir)/html/src


# maybe add above?  i guess not
# 	rm -rf $(docdir)


# register: install
# 	./setup register

register: install
	cabal register

doc: doc-clean haddock

hasktags=hasktags
# TEMP: hasktags is missing from http://hackage.haskell.org/platform/2010.1.0.0/
# hasktags=etags

hasktags:
	$(hasktags) *.cabal $(sources)

clean:
	rm -rf dist `$(find) . -name '*.hi' -o -name '*.o'` setup

# touch-setup:
# 	touch Setup.*hs

version ?= $(strip $(shell cat *.cabal | egrep -i '^version:' | sed -e 's/[Vv]ersion://'))

show-version:
	@echo $(version)

show-config-args:
	@echo $(configure-args)

show-build-args:
	@echo $(build-args)


# setup: Setup.*hs
# 	ghc --make Setup.*hs -o setup

# $(config-file): setup *.cabal
# 	./setup configure $(configure-args)

# modules with "." replaced by "/"
slash-modules = $(subst .,/,$(modules))
show-slash-modules:
	@echo $(slash-modules)

# Pattern to match .lhs, .hs and .hsc files.  I'd like a narrower pattern.  I
# can't use .hs*, since it'd include emacs backup files
source-suffixes = *h*[sc]

# Massage module names into source file names.
sources = $(patsubst %,$(top-src-dir)%.$(source-suffixes),$(slash-modules))
show-sources:
	@echo $(sources)

# Modify sources to use the unix end-of-line convention rather than dos.
unixify-sources:
	touch changes.tw TODO
	dos2unix README changes.tw Makefile TODO *.cabal Setup.*hs `$(find) $(top-src-dir) -name '*.*hs'`

# Or for docs (probably not worth bothering)
unixify-docs:
	dos2unix `$(find) dist/doc/html -name '*.html'`

# Similarly to get the dos end-of-line convention.
dosify-sources:
	unix2dos README CHANGES Makefile TODO *.cabal `$(find) $(top-src-dir) -name '*.*hs'`

# Remove old docs first, in case some modules have been renamed or deleted.
doc-clean:
	rm -rf dist/doc

# haddock-extra ?= --hyperlink-source --hscolour-css=$(hscolour-css)

haddock:
	cabal haddock $(haddock-extra)


# Determine version control system
VCS = $(shell if [ -d .git ]; then echo git; elif [ -d _darcs ]; then echo darcs; else echo none; fi)

show-vcs:
	@echo $(VCS)


# haddock: $(config-file)
#	./setup haddock $(haddock-extra)

ifeq ($(VCS),darcs)

# defaults for server locations.
server ?= darcs.haskell.org
server-dir ?= /home/darcs/packages
server-url-dir ?= /packages

endif

# For new projects, use code.haskell.org instead
# server = code.haskell.org
# server-dir = /srv/code
# server-url-dir =

url = http://$(server)$(server-url-dir)/$(package)

ssh = ssh $(user)@$(server)

ifeq ($(VCS),darcs)

# Make a new repo (requires account)
repo:
	darcs put $(user)@$(server):$(server-dir)/$(package)

endif

put: repo

tarball ?= $(package)-$(version).tar.gz

check-no-tarball:
	test ! -e tarballs/$(tarball) 

# tarball: build
# 	darcs dist -d $(package)-$(version)
# 	mkdir -p tarballs
# 	chmod a-w $(tarball)
# 	mv $(tarball) tarballs

# experiment
tarball: 
	 # build
	cabal sdist
	mkdir -p tarballs
	mv dist/$(tarball) tarballs
	chmod a-w tarballs/$(tarball)

# darcs-dist: tarball


# I make the .tar.gz unwritable to prevent the accident of forgetting to
# change the version number in a .cabal.


# After "darcs record" and "darcs push", see if everything works
test-get-build:
	cd $(tmp-dir); \
	rm -rf $(package); \
	darcs get --partial $(url); \
	cd $(package); \
	cabal configure --user; \
	cabal install

# 	runhaskell Setup configure --user; \
# 	runhaskell Setup build
# 	./setup build

#	ghc --make Setup.*hs; \
#	./setup configure; \
#	./setup build


# Server location for web docs
web-doc-dir ?= $(server-dir)/$(package)/doc/html

rm-web-doc:
	$(ssh) "rm -rf $(web-doc-dir)"

# The package's home on the web.
package-home ?= $(server):$(server-dir)/$(package)

# After "make doc", copy to the server.  Remove old docs first, in case
# some modules have been renamed or deleted.
# 
# Deprecated now that hackage is caught up with ghc.
deprecated-web-doc: rm-web-doc
	$(ssh) "mkdir -p $(web-doc-dir)"
	scp -r dist/doc/html/$(package)/* $(user)@$(package-home)/doc/html

# Check out local or remote docs.  firefox returns an error, erroniously
see-doc:
	$(BROWSER) dist/doc/html/$(package)/index.html; true
see-web-doc:
	$(BROWSER) $(url)/doc/html; true

# Make & see docs
doc-see: doc see-doc
web-doc-see: web-doc see-web-doc

# Automatically add yourself to the watch list of all project talk pages.
# There's one talk page for the project and one for each module.
watch-comments:
	$(BROWSER) "$(comments)&action=watch" ;\
	for file in $(slash-modules); do \
	  $(BROWSER) "$(comments)/$$file&action=watch"; \
	done; \
	echo Now watching all talk pages for $(package)

# alias
subscribe: watch-comments

# PanDoc.  Perhaps it makes more sense to put formatted info on the wiki.
%.html: %
	pandoc --toc --smart --standalone -o $@ $<

%.html: %.md
	pandoc --toc --smart --standalone -o $@ $<

pandocs: TODO.html CHANGES.html README.html

### Some simple targets here to help automate use from parent makefiles

ifeq ($(VCS),darcs)

# Check for forgotten darcs add
check-add:
	darcs add -r -v --dry-run .

add-new:
	darcs add -r -v .

tag:
	darcs tag $(version)

whatsnew:
	darcs whatsnew

push:
	darcs push --verbose

pull:
	darcs pull --verbose

record:
	darcs record

repair:
	darcs repair

release-record:
	darcs record --name="releasing version $(version)"

else ifeq ($(VCS),git)

tag:
	git tag -a $(version) -m "version $(version)"

release-record:
	git commit --all --message="releasing version $(version)" ; true

pull:
	git pull --verbose

push:
	git push --verbose

status:
	git status

endif

unreg:
	ghc-pkg unregister $(package)

unregister: unreg

force-unregister: 
	ghc-pkg unregister $(package) --force

# Handy for passing down to several libs  Generate & install one lib's
# docs before generating the next, so inter-lib links work.
haddock-install: haddock install

# command to upload your tarball.  you could pass in your username and
# password either in this variable or in a shell script readable only by
# you.
upload ?= cabal upload -v3

upload: manual-upload

manual-upload:
	$(upload) tarballs/$(package)-$(version).tar.gz

browser-upload:
	$(BROWSER) http://hackage.haskell.org/packages/upload.html; true

wiki:
	$(BROWSER) http://haskell.org/haskellwiki/$(package); true

see-hackage:
	$(BROWSER) http://hackage.haskell.org/package/$(package); true

# The whole release sequence.  For a first-time push, precede by "make repo"

# For the frequent case that the push fails (over satellite)
post-tag: push tarball upload see-hackage

post-haddock: release-record tag post-tag

release: check-no-tarball clean install haddock post-haddock

# TODO: restore haddock in release prereqs when my haddock & ghc are back
# in sync.

# If you want to update the wiki page, say "make wiki"

# Or use 'browser-upload' in place of 'manual-upload'

# removed time-consuming target test-get-build.
# also removed the pull (slow on my connection), so be careful!
# removed doc and web-doc-see, since hackage builds docs.
