Name:	    libmm-fileinfo
Summary:    Media Fileinfo
Version:    0.6.0
Release:    23
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(mm-common)
BuildRequires: pkgconfig(mm-log)
BuildRequires: pkgconfig(libswscale)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libavcodec)
BuildRequires: pkgconfig(libavutil)
BuildRequires: pkgconfig(libavformat)

%define use_drm 0

%if %{use_drm}
BuildRequires: libss-client-devel
BuildRequires: pkgconfig(drm-client)
%endif

%description
Multimedia Framework FileInfo Library


%package devel
Summary:    Media Fileinfo
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Multimedia Framework FileInfo Library (developement files)

%prep
%setup -q

%build
CFLAGS="${CFLAGS} -D_MM_PROJECT_FLOATER -DEXPORT_API=\"__attribute__((visibility(\\\"default\\\")))\" "
export CFLAGS

%autogen.sh

%if %{use_drm}
%configure  --disable-testmode --disable-dump --enable-dyn --disable-iommap --enable-drm --disable-gtk
%else
%configure --disable-testmode --disable-dump --enable-dyn --disable-iommap  --disable-drm --disable-gtk         
%endif

make

%install
%make_install
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest libmm-fileinfo.manifest
%{_bindir}/memtrace_reader
%{_bindir}/mm_file_test
%{_libdir}/libmmffile.so.*
%{_libdir}/libmmfile_codecs.so.*
%{_libdir}/libmmfile_formats.so.*
%{_libdir}/libmmfile_utils.so.*
%{_libdir}/libmmffile.so
%{_libdir}/libmmfile_codecs.so
%{_libdir}/libmmfile_formats.so
%{_libdir}/libmmfile_utils.so
%{_datadir}/license/%{name}

%files devel
%{_includedir}/mmf/mm_file.h
%{_libdir}/pkgconfig/mm-fileinfo.pc
