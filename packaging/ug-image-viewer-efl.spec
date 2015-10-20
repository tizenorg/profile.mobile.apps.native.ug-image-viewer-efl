%define cflags	" -Wall -Werror -Wextra -g -fPIC -O3 -Wno-unused-parameter -Wno-missing-field-initializers -fvisibility=hidden -finstrument-functions -Wl,--as-needed -fdata-sections -ffunction-sections -Wl,--gc-sections -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64"
%define cxxflags	" -Wall -Werror -Wextra -g -fPIC -O3 -Wno-unused-parameter -Wno-missing-field-initializers -fvisibility=hidden -fvisibility-inlines-hidden -finstrument-functions -Wl,--as-needed -fdata-sections -ffunction-sections -Wl,--gc-sections -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64"

%define _usrdir	/usr
%define _ugdir	%{_usrdir}/ug
%define _datadir /opt%{_ugdir}/data
%define _sharedir /opt/usr/media/.iv
%define _privatedatadir /opt/usr/apps/image-viewer-efl/data/

Name:       ug-image-viewer-efl
Summary:    Image Viewer UI Gadget v1.0
Version:    2.0.73
Release:    0
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires: cmake
BuildRequires: edje-tools
BuildRequires: gettext-tools
BuildRequires: prelink
BuildRequires: libicu-devel

BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(libmedia-utils)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(efreet)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(shortcut)
BuildRequires: pkgconfig(media-thumbnail)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(deviced)
#BuildRequires: pkgconfig(gstreamer-0.10)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(capi-appfw-preference)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-system-sensor)
BuildRequires: pkgconfig(capi-media-metadata-extractor)
BuildRequires: pkgconfig(capi-content-mime-type)
BuildRequires: pkgconfig(capi-media-player)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-media-sound-manager)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(sensor)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(storage)
%description
Description: Image Viewer UI Gadget v1.0

%prep
%setup -q

%build

CFLAGS+=%cflags
CXXFLAGS+=%cxxflags

%ifarch %{arm}
%define ARCH arm
CXXFLAGS+=" -D_ARCH_ARM_ -mfpu=neon "
CFLAGS+=" -D_ARCH_ARM_ -mfpu=neon "
%else
%define ARCH i586
%endif

export CXXFLAGS
export CFLAGS

cmake . -DCMAKE_INSTALL_PREFIX=%{_ugdir}/ -DCMAKE_DATA_DIR=%{_datadir} -DARCH=%{ARCH}

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}%{_sharedir}
mkdir -p %{buildroot}%{_privatedatadir}
cp LICENSE %{buildroot}/usr/share/license/ug-image-viewer-efl
execstack -c %{buildroot}%{_ugdir}/lib/libug-image-viewer-efl.so.0.1.0

%post
chown -R 5000:5000 %{_datadir}/ug-image-viewer-efl
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client /usr/ug/bin/image-viewer-efl

%files
%manifest ug-image-viewer-efl.manifest
%defattr(-,root,root,-)
%dir %{_datadir}/ug-image-viewer-efl

%defattr(-,root,root,-)
%{_ugdir}/lib/libug-image-viewer-efl.so*
%{_ugdir}/res/edje/ug-image-viewer-efl/*
%{_ugdir}/res/images/ug-image-viewer-efl/*
%{_ugdir}/res/locale/*/*/ug-image-viewer-efl.mo

/usr/share/license/ug-image-viewer-efl
/usr/share/packages/image-viewer-efl.xml
/usr/share/icons/default/small/ug-image-viewer-efl.png

%attr(775,root,root) /opt/etc/dump.d/module.d/dump-image-viewer.sh
%attr(775,app,app) %dir %{_sharedir}
%attr(777,app,app) %dir %{_privatedatadir}

%{_datadir}/ug-image-viewer-efl/*
