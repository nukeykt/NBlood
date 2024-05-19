#!/usr/bin/env python

import hashlib
import os
import shutil
import subprocess
import sys
import urllib2


class Package(object):
    prefix = ''
    environment = os.environ

    def __init__(self, name, source, checksum, arguments=()):
        self.name = name
        self.source = source
        self.checksum = checksum
        self.arguments = arguments

        self._filename = None
        self._work_path = None

    def build(self):
        print('=' * 80 + '\n Building ' + self.name + '\n' + '=' * 80)

        self._setup_workdir()

        configure_arguments = (
            './configure',
            '--enable-static',
            '--disable-shared',
            '--prefix=' + Package.prefix,
            '--disable-dependency-tracking'
        ) + self.arguments

        subprocess.check_call(configure_arguments, cwd=self._work_path, env=Package.environment)
        subprocess.check_call(('make', 'install'), cwd=self._work_path, env=Package.environment)

        self._work_path = None
        self._filename = None

    def _setup_workdir(self):
        assert not self._filename
        self._filename = self.source.rsplit('/', 1)[1]

        if os.path.exists(self._filename):
            checksum = _calculate_checksum(self._filename)
        else:
            checksum = self._download()

        if checksum != self.checksum:
            raise Exception("Checksum for %s doesn't match!" % self._filename)

        assert not self._work_path
        self._work_path = self._guess_work_path()
        self._extract()

    def _download(self):
        try:
            response = urllib2.urlopen(self.source)
        except urllib2.HTTPError, urllib2.URLError:
            request = urllib2.Request(self.source)
            request.add_header('User-Agent',
                               'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:60.0) Gecko/20100101 Firefox/60.0')
            opener = urllib2.build_opener()
            response = opener.open(request)

        checksum = hashlib.sha256()
        step = 64 * 1024
        total = 0

        try:
            with open(self._filename, 'wb') as f:
                while True:
                    data = response.read(step)
                    total += len(data)

                    if not data:
                        sys.stdout.write('\n')
                        return checksum.hexdigest()

                    f.write(data)
                    checksum.update(data)

                    sys.stdout.write('\rDownloading %s: %i bytes' % (self._filename, total))
                    sys.stdout.flush()
        except IOError:
            os.unlink(self._filename)
            raise

    def _guess_work_path(self):
        files = subprocess.check_output(['tar', '-tf', self._filename])
        result = ''
        shortest = sys.maxint

        for name in files.split('\n'):
            parts = name.split('/')
            parts_count = len(parts)

            if 'configure' == parts[-1]:
                if parts_count < shortest:
                    result = '/'.join(parts[:-1])
                    shortest = parts_count

        return result

    def _extract(self):
        try:
            subprocess.check_call(['tar', '-xf', self._filename])
        except (IOError, subprocess.CalledProcessError):
            shutil.rmtree(self._work_path, ignore_errors=True)
            raise


def _calculate_checksum(filename):
    checksum = hashlib.sha256()

    with open(filename, 'rb') as f:
        data = True

        while data:
            data = f.read(64 * 1024)
            checksum.update(data)

    return checksum.hexdigest()


def _setup_environment():
    root_path = os.path.dirname(os.path.abspath(__file__))
    build_path = root_path + os.sep + 'depsbuild'

    if not os.path.exists(build_path):
        os.mkdir(build_path)

    os.chdir(build_path)

    Package.prefix = root_path

    macos_min_ver = '10.9'
    common_flags = '-mmacosx-version-min=' + macos_min_ver

    sdk_path = build_path + os.sep + 'MacOSX' + macos_min_ver + '.sdk'
    if os.path.exists(sdk_path):
        common_flags += ' -isysroot ' + sdk_path

    cflags = common_flags + ' -I' + root_path + os.sep + 'include'
    ldflags = common_flags + ' -L' + root_path + os.sep + 'lib'

    Package.environment['CPPFLAGS'] = cflags
    Package.environment['CFLAGS'] = cflags
    Package.environment['CXXFLAGS'] = cflags
    Package.environment['OBJCFLAGS'] = cflags
    Package.environment['OBJCXXFLAGS'] = cflags
    Package.environment['LDFLAGS'] = ldflags


_packages = (
    Package(
        name='FLAC',
        source='https://downloads.xiph.org/releases/flac/flac-1.3.2.tar.xz',
        checksum='91cfc3ed61dc40f47f050a109b08610667d73477af6ef36dcad31c31a4a8d53f',
        arguments=('--disable-cpplibs',)
    ),
    Package(
        name='Yasm',
        source='https://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz',
        checksum='3dce6601b495f5b3d45b59f7d2492a340ee7e84b5beca17e48f862502bd5603f',
    ),
    Package(
        name='libvpx',
        source='https://github.com/webmproject/libvpx/archive/v1.8.0.tar.gz',
        checksum='86df18c694e1c06cc8f83d2d816e9270747a0ce6abe316e93a4f4095689373f6',
        arguments=('--disable-examples', '--disable-unit-tests')
    ),
    Package(
        name='SDL2',
        source='https://www.libsdl.org/release/SDL2-2.0.9.tar.gz',
        checksum='255186dc676ecd0c1dbf10ec8a2cc5d6869b5079d8a38194c2aecdff54b324b1',
        arguments=('--without-x',)
    ),
)


def _main():
    _setup_environment()

    for package in _packages:
        package.build()


if __name__ == '__main__':
    _main()
