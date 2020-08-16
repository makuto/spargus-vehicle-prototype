#!/bin/sh

curl https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_linux_c%2B%2Blatest.sh > Dependencies/DownloadBuildOgre_Linux.sh

cd Dependencies
chmod +x DownloadBuildOgre_Linux.sh
./DownloadBuildOgre_Linux.sh

echo "Copying default data files"
rsync -av Dependencies/Ogre/ogre-next/Samples/Media/Hlms/Common data/Hlms/
rsync -av Dependencies/Ogre/ogre-next/Samples/Media/Hlms/Pbs data/Hlms/
rsync -av Dependencies/Ogre/ogre-next/Samples/Media/Hlms/Unlit data/Hlms/
rsync -av Dependencies/Ogre/ogre-next/Samples/Media/2.0/scripts/materials/Common data/CommonMaterials
