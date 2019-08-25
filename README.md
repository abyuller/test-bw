# test-bw

The goal of the project is to demonstrate the use of the C language, MySQL, Memcache and FastCGI in the multithreading mode.

## Getting Started

For the further development of the project, install the following packages:

*gcc*

*make*

*libmysqlclient-dev* (Ubuntu) or *mariadb-clients* (Archlinux)

*libmemcached-dev* (Ubuntu) or *libmemcached* (Archlinux)

*libfcgi-dev* (Ubuntu) or *fcgi* (Archlinux)

For the testing and debugging on the standalone PC install next packages:

*nginx*

*mysql*

*memcached*

### Installing

Type next commands:

*cd PATH_TO_test-bw/src*

*mkdir ../obj*

*make*

*cd ../bin*

Change the settings in the file to suit your requirements.

*nano test-bw.conf*

Enter the next command to start the program:

*./test-bw*

Type Ctrl+C to stop the program.

## Running the tests

No automated tests have been applied yet. Only debugging.

### And coding style tests

No coding style tests have been applied yet.

## Deployment

On a live linux system, you need the following packages

*memcached-client*

*mysql-client*

*fcgi*

This application listen the local port 9000. If you do not like this, correct the port number in the file *test-bw.c* and rebuild the project.


## Built With

* [gcc](https://gcc.gnu.org/) - The GNU Compiler Collection used

## Versioning

I have not used any tools for versioning. And the project also has no version.

## Authors

* **Andrew Byuller** - *JSC TEMZ* - [abyuller](https://github.com/abyuller)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## External links

[Web-приложение на C/C++ с помощью FastCGI — это просто](https://habr.com/ru/post/154187/)

[README-Template.md](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2)

