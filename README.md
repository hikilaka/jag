# Jag

Jag is a command line utility to manage Jagex's proprietary cache files. It can extract or insert files into existing archives, or even create brand new archives.

## Usage

Using jag is easy. Just specify the name of the files you want to extract and the archive's name.
```bash
$ jag --extract logo.tga jagex.jag
$ jag --extract logo.tga
```
You can create a new archive with the --create option.
```bash
$ jag --create sysdevs.jag --insert something_cool.txt
```
Try `$ jag --help` for additional usage information.

## Dependencies

* Any C++17 compiler
* Boost (pretty much any some-what modern version will work)

## License
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program. If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).

