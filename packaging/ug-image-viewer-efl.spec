
Name:       org.tizen.image-viewer
Summary:    image-viewer
Version:    2.0.73
Release:    1
Group:      Applications/Multimedia Applications
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
BuildRequires: pkgconfig(efreet)
BuildRequires: pkgconfig(mm-fileinfo)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(shortcut)
BuildRequires: pkgconfig(media-thumbnail)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(deviced)
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
BuildRequires: pkgconfig(libtzplatform-config)

%description
Myfile Application v1.0.
%define _smack_domain %{name}


%description
Description: Image Viewer

%define PREFIX    	 %{TZ_SYS_RO_APP}/%{name}
%define MANIFESTDIR      %{TZ_SYS_RO_PACKAGES}
%define ICONDIR          %{TZ_SYS_RO_ICONS}/default/small

%define RESDIR           %{PREFIX}/res
%define EDJDIR           %{RESDIR}/edje
%define IMGDIR           %{RESDIR}/images
%define BINDIR           %{PREFIX}/bin
%define LIBDIR           %{PREFIX}/lib
%define LOCALEDIR        %{RESDIR}/locale

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . \
    -DPREFIX=%{PREFIX}   \
    -DPKGDIR=%{name}     \
    -DIMGDIR=%{IMGDIR}   \
    -DEDJDIR=%{EDJDIR}   \
    -DPKGNAME=%{name}    \
    -DBINDIR=%{BINDIR}   \
    -DMANIFESTDIR=%{MANIFESTDIR}   \
    -DEDJIMGDIR=%{EDJIMGDIR}   \
    -DLIBDIR=%{LIBDIR}   \
    -DICONDIR=%{ICONDIR}   \
    -DLOCALEDIR=%{LOCALEDIR}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{LIBDIR}

%post
GOPTION="-g 6514"

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%dir
%{LIBDIR}
%{BINDIR}/*
%{MANIFESTDIR}/*.xml
%{ICONDIR}/*
%{RESDIR}/*
