# Memray tests

Install memray (only macOS and Linux supported):

```console
$ sudo apt-get install libunwind-dev
$ sudo apt-get install liblz4-dev
$ pip install memray
```

Run memray on each file and start the HTTP server on `localhost:8000`:

```console
$ make profiles
```

Once the web server is running, look at the HTML files to identify any leaks.

To create a copy of the profiles (e.g before making fixes):

```console
$ make snapshot
```

This will copy the `.profiles` folder to `.profiles-snapshot`

To cleanup profiles:

```console
$ make clean
```
