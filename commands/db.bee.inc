<?php
/**
 * @file
 * Command(s) for working with Backdrop's database.
 */

/**
 * Implements hook_bee_command().
 */
function db_bee_command() {
  return array(
    'db-export' => array(
      'description' => bt('Export the database as a compressed SQL file.'),
      'callback' => 'db_export_bee_callback',
      'arguments' => array(
        'file' => bt('The SQL file where the exported database will be saved. Leave blank to use the current date/time as the filename.'),
      ),
      'optional_arguments' => array('file'),
      'options' => array(
        'column-statistics' => array(
          'description' => bt("Allow extra options to be passed to the MySQL dump command."),
          'value' => bt('String'),
        ),
        'no-tablespaces' => array(
          'description' => bt("Allow extra options to be passed to the MySQL dump command."),
          'value' => bt('String'),
        ),
      ),
      'aliases' => array('dbex', 'db-dump', 'sql-export', 'sql-dump'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee db-export db.sql' => bt('Export the database to db.sql.gz.'),
        'bee db-export' => bt('Export the database to [DATE_TIME].sql.gz.'),
      ),
    ),
    'db-import' => array(
      'description' => bt('Import an SQL file into the current database.'),
      'callback' => 'db_import_bee_callback',
      'arguments' => array(
        'file' => bt('The SQL file to import into the database. Can be gzipped (i.e. *.sql.gz).'),
      ),
      'aliases' => array('dbim', 'sql-import'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee db-import backup.sql' => bt('Import backup.sql into the current database.'),
        'bee db-import db.sql.gz' => bt('Extract and import db.sql into the current database.'),
      ),
    ),
    'db-drop' => array(
      'description' => bt('Drop the current database and recreate an empty database with the same details. This could be used prior to import if the target database has more tables than the source database.'),
      'callback' => 'db_drop_bee_callback',
      'aliases' => array('sql-drop'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee db-drop' => bt('Drop the current database and recreate an empty database with the same details.'),
      ),
      'options' => array(
        'y' => array(
          'description' => bt("A database connection string of the form: 'mysql://user:pass@localhost/database_name'. See https://api.backdropcms.org/database-configuration for more details."),
          'value' => bt('String'),
        ),
      ),
    ),
    'sql' => array(
      'description' => bt("Open an SQL command-line interface using Backdrop's database credentials."),
      'callback' => 'sql_bee_callback',
      'aliases' => array('sqlc', 'sql-cli', 'db-cli'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee sql' => bt('Open a command-line for the current database.'),
        'bee sql < backup.sql' => bt('Import an SQL file into the current database.'),
      ),
    ),
  );
}

/**
 * Command callback: Export the database as a compressed SQL file.
 */
function db_export_bee_callback($arguments, $options) {
  global $_bee_backdrop_root;

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get the filename for the export.
  $filename = isset($arguments['file']) ? $arguments['file'] : date('Ymd_His') . '.sql';

  // Added handling for these two options. Note that trying to mimic "extra-dumps" as per
  // drush does not work due to the way arguments and options are parsed. (quotes not respected)
  // These are needed for some configurations of MariaDB/MySQL to work.
  $colstats = (!empty($options['column-statistics']) || $option['column-statistics'] == '0') ? '--column-statistics=' . $options['column-statistics'] : NULL;
  $tablespaces = (!empty($options['no-tablespaces'])) ? '--no-tablespaces' : NULL;

  // Export and compress the database.
  $export_command = 'mysqldump --user=' . $db_info['username'] . ' --password=' . $db_info['password'] . ' ' . $colstats . ' ' . $tablespaces . '  --host=' . $db_info['host'] . ' ' . $db_info['database'] . ' | gzip > ' . $filename . '.gz';
  exec($export_command, $output, $result);

  if ($result === 0) {
    bee_message(bt("The '!database' database was exported to '!filename.gz'.", array(
      '!database' => $db_info['database'],
      '!filename' => $_bee_backdrop_root . '/' . $filename,
    )), 'success');
  }
}

/**
 * Command callback: Import an SQL file into the current database.
 */
function db_import_bee_callback($arguments, $options) {
  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get the file for the import.
  $filename = $arguments['file'];

  // Determine if this is a compressed file.
  $gzip = FALSE;
  if (substr($filename, -3) == '.gz') {
    $gzip = TRUE;
  }

  $db_username = rawurldecode($db_info['username']);
  $db_password = rawurldecode($db_info['password']);
  $db_database = rawurldecode($db_info['database']);

  // Import the database.
  $import_command = '';
  if ($gzip) {
    $import_command .= "gunzip -c $filename | ";
  }
  $import_command .= 'mysql --user=' . $db_username . ' --password=' . $db_password . ' --host=' . $db_info['host'] . ' ' . $db_database;
  if (!$gzip) {
    $import_command .= " < $filename";
  }
  exec($import_command, $output, $result);

  if ($result === 0) {
    bee_message(bt("'!filename' was imported into the '!database' database.", array(
      '!filename' => $filename,
      '!database' => $db_info['database'],
    )), 'success');
  }
}

/**
 * Command callback: Drop and re-create the existing configured database.
 */
function db_drop_bee_callback($arguments, $options) {
  // Initialize our pipes variable.
  $pipes = array();

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get database connection string.
  if (empty($options['y'])) {
    // Prompt to continue.
    if (!bee_confirm(bt('Are you sure you want to drop the database !database ?', array(
        '!database' => $db_info['database']
      )), FALSE)) {
      return;
    }
  }

  $db_username = rawurldecode($db_info['username']);
  $db_password = rawurldecode($db_info['password']);
  $db_database = rawurldecode($db_info['database']);

  // Drop the existing backdrop database as configured.
  $command = 'mysql --user=' . $db_username . ' --password=' . $db_password . ' --host=' . $db_info['host'] . ' -e "drop database ' . $db_database . '"';
  $result = proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));

  if ($result == -1) {
    bee_message(bt("The '!database' database could not be dropped.", array(
      '!database' => $db_info['database'],
    )), 'error');
  }
  else {
    bee_message(bt("The '!database' database was successfully dropped.", array(
      '!database' => $db_info['database'],
    )), 'success');
  }

  // Re-create the existing backdrop database as configured.
  $command = 'mysql --user=' . $db_username . ' --password=' . $db_password . ' --host=' . $db_info['host'] . ' -e "create database ' . $db_database . '"';
  $result = proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));

  if ($result == -1) {
    bee_message(bt("The '!database' database could not be created.", array(
      '!database' => $db_info['database'],
    )), 'error');
  }
  else {
    bee_message(bt("The '!database' database was successfully created.", array(
      '!database' => $db_info['database'],
    )), 'success');
  }
}

/**
 * Command callback: Open an SQL command-line interface using Backdrop's
 * database credentials.
 */
function sql_bee_callback($arguments, $options) {
  // Initialize our pipes variable.
  $pipes = array();

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  $db_username = rawurldecode($db_info['username']);
  $db_password = rawurldecode($db_info['password']);
  $db_database = rawurldecode($db_info['database']);

  // Open SQL command-line.
  $command = 'mysql --user=' . $db_username . ' --password=' . $db_password . ' --host=' . $db_info['host'] . ' ' . $db_database;
  proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));
}
