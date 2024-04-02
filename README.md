# Traffic usage
It is a simple program that collects statistics about your internet usage and stores them in a file. It depends on `ncurses`

# Build and usage
```
cmake -B build
cmake --build build --target traffic_usage
sudo cmake --install build
traffic_usage --help
```
By default the program will be installed to `/usr/local/`, you can change the installation dir
specifying a `--prefix` in the last command

At first, run the daemon that will collect statistics: `traffic_usage -d <interface>`
where `<interface>` is an interface to collect statistics (to see list of all interfaces on your
system use: `traffic_usage -l`)

To kill the daemon use: `traffic_usage -k`

# TUI
You can see the statistics using: `traffic_usage -t`

* `h` - previous month
* `l` - next month
* `j` - scroll down
* `k` - scroll up
