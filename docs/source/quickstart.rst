Quickstart
==========

Run a Configured *ns-3* Scenario
--------------------------------
Run a scenario that uses the `netsimulyzer companion module <https://github.com/usnistgov/NetSimulyzer-ns3-module>`_.

Download the Application
------------------------
Download the prebuilt application for your platform from the
`releases section on GitHub <https://github.com/usnistgov/NetSimulyzer/releases>`_.

For macOS, the application must be built from source, see :ref:`building-for-macos` for
more information, and skip to :ref:`run-the-application`.

Decompress the Application
---------------------------
On Windows, right-click on the ZIP file and select 'Extract All'

On Linux run the following command, replacing 'version' with the downloaded version:

.. code-block:: Bash

  tar xf Linux-version.tar.xz

.. _run-the-application:

Run the Application
-------------------
On Windows, enter the new folder and double click 'visualization.exe'. N.B. The '.exe' extension
may be hidden per-user settings.

On Linux and macOS, enter the 'NetSimulyzer' directory, and run the application like so:

.. code-block:: Bash

  cd NetSimulyzer
  ./netsimulyzer

Open the *ns-3* Module Output File
----------------------------------
Go to 'File' > 'Load', or press ``Ctrl + O`` (Windows and Linux) or ``command + O`` (macOS).
Navigate with the dialog that opens to the JSON file which was output from the *ns-3* scenario, select it,
and click 'Open'.

In a few moments, the message at the bottom of the window should change from 'Loading' to
'Successfully loaded' along with the *ns-3* output filename. This indicates the scenario
may now be replayed.

Replaying the Scenario
----------------------
Once the scenario has fully loaded, it can be replayed by pressing the 'Play' button located at
the top of the window (Default), or pressing the 'Play/Pause' key (``P`` by default). For more
information about the application controls, see: :doc:`controls`.
