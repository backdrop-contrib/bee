#!/bin/sh
###
## Shell script to setup Bee's testing environment.
###

### Clean-up tasks that get the system back to a clean slate.
clean_up() {
  cd /app

  # Remove Backdrop installations.
  rm -rf backdrop/ multisite/ current/ directory/ defined_release/

  # Drop databases.
  mysql -h database -u root -e "DROP DATABASE backdrop;"
  mysql -h database -u root -e "DROP DATABASE multi_one;"
  mysql -h database -u root -e "DROP DATABASE multi_two;"
  mysql -h database -u root -e "DROP DATABASE install_test;"
}

### Setup tasks that need to run on a clean slate.
set_up() {
  cd /app

  # Create databases.
  mysql -h database -u root -e "CREATE DATABASE IF NOT EXISTS backdrop; GRANT ALL PRIVILEGES ON backdrop.* TO 'backdrop'@'%' IDENTIFIED by 'backdrop';"
  mysql -h database -u root -e "CREATE DATABASE multi_one; GRANT ALL PRIVILEGES ON multi_one.* TO 'backdrop'@'%' IDENTIFIED by 'backdrop';"
  mysql -h database -u root -e "CREATE DATABASE multi_two; GRANT ALL PRIVILEGES ON multi_two.* TO 'backdrop'@'%' IDENTIFIED by 'backdrop';"
  mysql -h database -u root -e "CREATE DATABASE install_test; GRANT ALL PRIVILEGES ON install_test.* TO 'backdrop'@'%' IDENTIFIED by 'backdrop';"
  mysql -h database -u root -e "FLUSH PRIVILEGES;"

  # Configure Backdrop installation.
  unzip -q backdrop.zip && mv backdrop-1.x backdrop
  echo 'if (isset($_SERVER["BACKDROP_SETTINGS"])) unset($_SERVER["BACKDROP_SETTINGS"]);' >> backdrop/settings.php

  # Disable sending of telemetry data from GitHub Action runners. Override if
  # testing telemetry locally.
  echo '$settings["telemetry_enabled"] = FALSE;' >> backdrop/settings.php

  # Configure multisite installation.
  cp -r backdrop multisite
  cd multisite
  echo '$sites["multi-1.lndo.site"] = "multi_one";' >> sites/sites.php
  echo '$sites["multi-2.lndo.site"] = "multi_two";' >> sites/sites.php
  echo '$sites["install-test.lndo.site"] = "install_test";' >> sites/sites.php
  mkdir sites/multi_one && cp settings.php sites/multi_one/settings.php
  mkdir sites/multi_two && cp settings.php sites/multi_two/settings.php
  mkdir sites/install_test && cp settings.php sites/install_test/settings.php

  # Install sites.
  cd ../backdrop
  ./core/scripts/install.sh --account-pass=password --db-url=mysql://backdrop:backdrop@database/backdrop
  cd ../multisite
  ./core/scripts/install.sh --url=multi-1.lndo.site --account-pass=password --site-name="Multisite 1" --db-url=mysql://backdrop:backdrop@database/multi_one
  ./core/scripts/install.sh --url=multi-2.lndo.site --account-pass=password --site-name="Multisite 2" --db-url=mysql://backdrop:backdrop@database/multi_two
  # The `install_test` site is only installed during test runs, not here.
}

### Run different tasks depending on any arguments passed to the script.
if [ -z $1 ]; then
  # No argument, run everything.
  clean_up
  set_up
else
  if [ $1 = 'clean' ]; then
    # Just clean-up.
    clean_up
  else
    # Just setup.
    set_up
  fi
fi
