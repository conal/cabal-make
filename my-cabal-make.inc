# Example personal cross-project settings.
# You can copy, customize, and use instead of cabal-make.inc.

user = conal

# Windows-style.  If this path contains spaces, you are best off using the
# 8.3-style version of the path (dir /x can be used to figure this out)
# 
# cabal-make = c:/conal/Haskell/cabal-make
# configure-dirs = --prefix=c:/Haskell/packages --datadir=c:/Haskell/packages --libdir=c:/Haskell/packages --bindir=c:/Haskell/packages/bin

# *nix style:
cabal-make = /Users/conal/Haskell/cabal-make
extra-configure-args += --user

hscolour-css = $(cabal-make)/hscolour.css

# Generic settings:
include $(cabal-make)/cabal-make.inc
