# Copyright 1999-2015 Gentoo Foundation 
# Distributed under the terms of the GNU General Public License v2

EAPI="5"
inherit  cmake-utils git-r3

DESCRIPTION="Softphone for VoIP communcations using SIP protocol"
HOMEPAGE="http://twinkle.dolezel.info/"
# aixtema admins github
EGIT_REPO_URI="https://github.com/aixtemaadmins/twinkle.git"

LICENSE="GPL-2"
SLOT="0"
IUSE="alsa speex ilbc zrtp  +qt4 qt5 g729 diamondcard "

RDEPEND="media-libs/fontconfig
    g729? ( media-plugins/mediastreamer-bcg729 )
    dev-libs/boost
    speex? ( media-libs/speex )
    ilbc? ( dev-libs/ilbc-rfc3951 )
    zrtp? ( net-libs/libzrtpcpp )
    media-libs/alsa-lib
    dev-cpp/commoncpp2
    dev-libs/ucommon
    media-libs/libsndfile
    "
REQUIRED_USE="
    ?? ( qt4 qt5 )
"

src_configure() {
 local mycmakeargs=(
    -DCMAKE_INSTALL_PREFIX=/usr/local
    $(cmake-utils_use_with alsa ALSA)
    $(cmake-utils_use_with speex SPEEX)
    $(cmake-utils_use_with ilbc ILBC)
    $(cmake-utils_use_with zrtp ZRTP)
    $(cmake-utils_use_with qt4 QT4)
    $(cmake-utils_use_with qt5 QT5)
    $(cmake-utils_use_with diamondcard DIAMONDCARD)
    $(cmake-utils_use_with g729 G729)
    )
    cmake-utils_src_configure
}

S="${WORKDIR}"/${P}
