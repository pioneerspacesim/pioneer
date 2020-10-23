#!/c/msys64/usr/bin/bash -l

# Exit immediately upon error
set -e

cd /c/projects
git clone --depth 1 git://github.com/pioneerspacesim/pioneer-thirdparty
cd /c/projects/pioneer

# Get some languages for Inno Setup that aren't officially supported/updated in version 6
curl -o "/c/Program Files (x86)/Inno Setup 6/Languages/Greek.isl" https://raw.githubusercontent.com/jrsoftware/issrc/main/Files/Languages/Unofficial/Greek.isl
curl -o "/c/Program Files (x86)/Inno Setup 6/Languages/Hungarian.isl" https://raw.githubusercontent.com/jrsoftware/issrc/main/Files/Languages/Unofficial/Hungarian.isl
curl -o "/c/Program Files (x86)/Inno Setup 6/Languages/ScottishGaelic.isl" https://raw.githubusercontent.com/jrsoftware/issrc/main/Files/Languages/Unofficial/ScottishGaelic.isl
curl -o "/c/Program Files (x86)/Inno Setup 6/Languages/SerbianCyrillic.isl" https://raw.githubusercontent.com/jrsoftware/issrc/main/Files/Languages/Unofficial/SerbianCyrillic.isl
curl -o "/c/Program Files (x86)/Inno Setup 6/Languages/SerbianLatin.isl" https://raw.githubusercontent.com/jrsoftware/issrc/main/Files/Languages/Unofficial/SerbianLatin.isl
