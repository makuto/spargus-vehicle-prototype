#!/bin/sh

cp ../ogre2-template/*.cfg .
sed -i "s+=Ogre+=../ogre2-template/Ogre+g" plugins.cfg resources2.cfg
