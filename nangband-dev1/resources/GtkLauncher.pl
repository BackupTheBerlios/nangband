#!/usr/bin/perl -w
# Nangband launcher

use Gtk '-init';
use strict;

# Change this to the location of your variant
my $angband_path = "~/Projects/Angbands/nangband/nangband";

# We make all the stuff we need here; a window, four buttons and a VBox.
my $window = new Gtk::Window;
my $button_quit = new Gtk::Button("Quit");
my $button_gtk = new Gtk::Button("GTK mode");
my $button_x11 = new Gtk::Button("X11 mode");
my $button_gcu = new Gtk::Button("GCU mode");
my $window_main_vbox = new Gtk::VBox(0, 4);

# Set the window title; change this if you want.
$window->set_title("Nangband Launcher");
$window->set_border_width(5);

# Connect the clicking event to the buttons.
$button_gtk->signal_connect("clicked", sub { `$angband_path -mgtk`; });
$button_x11->signal_connect("clicked", sub { `$angband_path -mx11`; });
$button_gcu->signal_connect("clicked", sub { `xterm -e $angband_path -mgcu`; });
$button_quit->signal_connect("clicked", sub {Gtk->main_quit});

# Add the VBox to the window
$window->add($window_main_vbox);

# Add the items to the VBox
$window_main_vbox->add($button_gtk);
$window_main_vbox->add($button_x11);
$window_main_vbox->add($button_gcu);
$window_main_vbox->add($button_quit);

# Show all
$window->show_all;

# Wait
Gtk->main;
