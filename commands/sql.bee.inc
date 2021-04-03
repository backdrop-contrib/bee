<?php
/**
 * @file
 * Command(s) for sqling Backdrop projects.
 */

/**
 * Implements hook_bee_command().
 */
function sql_bee_command() {
  return array(
    'sql-dump' => array(
      'description' => bt('Exports the Backdrop DB as SQL using mysqldump.'),
      'callback' => '_sql_dump_execute',
      'options' => array(
        'result-file' => array(
          'description' => bt('Save to a file. The file should be relative to Backdrop root. If empty --result-file is provided, then date based filename will be created under the document root of Backdrop site.'),
        ),
        'gzip' => array(
          'description' => bt('Compress the dump using the gzip program which must be in your $PATH.'),
        ),
      ),
      'examples' => array(
        'bee sql-dump --result-file' => bt('Save SQL dump into the date-based file name in the Backdrop root.'),
        'bee sql-dump --result-file --gzip' => bt('Compress the SQL dump.'),
        'bee sql-dump --result-file=../example.sql' => bt('Save SQL dump to the directory above the Backdrop root.'),
        'bee sql-dump' => bt('Output SQL dump to the screen.'),
        'bee sql-dump > example.sql' => bt('Save SQL dump to the current working directory.'),
      ),
    ),
    'sql-cli' => array(
      'description' => bt('Open a SQL command-line interface using Backdrop\'s credentials.'),
      'callback' => '_sql_cli_execute',
      'aliases' => array('sqlc'),
      'examples' => array(
        'bee sql-cli < example.sql' => bt('Import sql statements from a file into the current database.'),
      ),
    ),
    'sql-drop' => array(
      'description' => bt('Drop all tables in a given database.'),
      'callback' => '_sql_drop_execute',
      'options' => array(
        'yes' => array(
          'description' => bt('Skip confirmation and proceed.'),
        ),
        'result-file' => array(
          'description' => bt('Save to a file. The file should be relative to Backdrop root.'),
        ),
      ),
    ),
  );
}

/**
 * Outputs the entire Backdrop database in SQL format using mysqldump.
 */
function _sql_dump_execute($arguments, $options) {
  global $_bee_backdrop_root;
  if (!empty($_bee_backdrop_root)) {
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      $db = array_shift($db_connection);
      $query = "mysqldump --user={$db['username']} --password={$db['password']} --host={$db['host']} {$db['database']}";

      if (isset($options['result-file'])) {
        // Accidental `bee sql-dump --result-file` without file name returns 1.
        if (empty($options['result-file']) || $options['result-file'] == '1') {
          $result_file = time() . '.sql';
        }
        else {
          $result_file = $options['result-file'];
        }
        if (isset($options['gzip'])) {
          shell_exec($query . ' | gzip -c > ' . getcwd() . '/' . $result_file . '.gz');
        }
        else {
          shell_exec($query . ' > ' . getcwd() . '/' . $result_file);
        }
        bee_message(bt('The database has been succesfully dumped into @file', array(
          '@file' => $result_file,
        )), 'success');
      }
      else {
        // The 'result-file' option was not entered, so output dump to screen.
        $result = shell_exec($query);
        print_r($result);
      }
    }
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}

/**
 * Opens a SQL command-line interface using Backdrop's credentials..
 */
function _sql_cli_execute() {
  global $_bee_backdrop_root;
  if (!empty($_bee_backdrop_root)) {
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      $db = array_shift($db_connection);
      $query = "mysql --user={$db['username']} --password={$db['password']} --host={$db['host']} {$db['database']}";
      exec($query);
    }
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}

/**
 * Drops all tables in a given database..
 */
function _sql_drop_execute() {
  global $_bee_backdrop_root;
  if (!empty($_bee_backdrop_root)) {
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      $db = array_shift($db_connection);
      if (!bee_confirm(bt('Do you really want to drop all tables?'))) {
        return;
      }
      $connection = mysqli_connect($db['host'], $db['username'], $db['password']);
      // Drop Database
      $query = "DROP DATABASE `{$db['database']}`";
      if ($connection->query($query) === TRUE) {
        // Re-create the database.
        $query = "CREATE DATABASE `{$db['database']}`";
        $connection->query($query);
      } else {
        bee_message(bt('Unable to drop database ' . $connection->error, array(
          '@database' => $db['database'],
        )), 'error');
      }
      $connection->close();
    }
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}
