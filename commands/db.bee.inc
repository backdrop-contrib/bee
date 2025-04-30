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
      'description' => bt('Export the database as a compressed SQL file. This uses the --no-tablespaces option by default.'),
      'callback' => 'db_export_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => TRUE,
      ),
      'group' => 'database',
      'arguments' => array(
        'file' => bt('The SQL file where the exported database will be saved. Leave blank to use the current date/time as the filename.'),
      ),
      'optional_arguments' => array('file'),
      'options' => array(
        'extra' => array(
          'description' => bt("Add custom option(s) to the 'mysqldump' command. Do not include leading or trailing spaces and enclose multiple options in double quotes. '--no-tablespaces' option is not used unless you add it."),
          'value' => bt('Custom mysqldump options'),
        ),
      ),
      'aliases' => array('dbex', 'db-dump', 'sql-export', 'sql-dump'),
      'examples' => array(
        'bee db-export db.sql' => bt('Export the database to db.sql.gz.'),
        'bee db-export' => bt('Export the database to [DATE_TIME].sql.gz.'),
        'bee db-export --extra=" " db.sql' => bt("Export the database to db.sql.gz without using the '--no-tablespaces' option."),
        'bee db-export --extra="--no-data --no-tablespaces" db.sql' => bt("Export the database without data, and using the '--no-tablespaces' option, to db.sql.gz."),
      ),
    ),
    'db-import' => array(
      'description' => bt('Import an SQL file into the current database.'),
      'callback' => 'db_import_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'database',
      'arguments' => array(
        'file' => bt('The SQL file to import into the database. Can be gzipped (i.e. *.sql.gz).'),
      ),
      'options' => array(
        'mariadb-compatibility' => array(
          'description' => bt('Recent MariaDB versions have started including a command to enable sandbox mode, which can cause issues for importing on older versions or into MySQL. Use this option to remove that string before import.'),
          'short' => 'mdbc',
        ),
      ),
      'aliases' => array('dbim', 'sql-import'),
      'examples' => array(
        'bee db-import backup.sql' => bt('Import backup.sql into the current database.'),
        'bee db-import db.sql.gz' => bt('Extract and import db.sql into the current database.'),
        'bee db-import db.sql.gz --mariadb-compatibility' => bt('Extract and import db.sql into the current database removing any MariaDB sandbox switch first.'),
      ),
    ),
    'db-drop' => array(
      'description' => bt('Drop the current database and recreate an empty database with the same details. This could be used prior to import if the target database has more tables than the source database.'),
      'callback' => 'db_drop_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'database',
      'aliases' => array('sql-drop'),
      'examples' => array(
        'bee db-drop' => bt('Drop the current database and recreate an empty database with the same details.  You will then be prompted to confirm.'),
        'bee --yes db-drop' => bt('Drop the current database and recreate an empty database with the same details.  You will NOT be prompted to confirm.'),
      ),
    ),
    'sql' => array(
      'description' => bt("Open an SQL command-line interface using Backdrop's database credentials."),
      'callback' => 'sql_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'advanced',
      'aliases' => array('sqlc', 'sql-cli', 'db-cli'),
      'examples' => array(
        'bee sql' => bt('Open a command-line for the current database.'),
        'bee sql < ../backup.sql' => bt('Use SQL statements in a file to import data into the current database.'),
        'bee sql < ../scripts/sanitize.sql' => bt('Use SQL statements in a file to complete operations with the current database.'),
      ),
    ),
    'db-query' => array(
      'description' => bt('Execute a query using db_query().'),
      'callback' => 'dbq_bee_callback',
      'group' => 'advanced',
      'arguments' => array(
        'query' => bt("An SQL query as passed to db_query() (no placeholders)."),
      ),
      'aliases' => array('dbq'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee db-query "SELECT * FROM {users} WHERE uid = 1"' => bt('Browse user record. Note that table prefixes are honored.'),
        'bee db-query "UPDATE {users} SET mail = \'me@example.com\' WHERE uid = 1"' => bt('Update a user\'s email address.'),
        'bee db-query "DELETE FROM {users} WHERE uid = 2"' => bt('Delete a user record from the db.'),
        'bee db-query "SELECT name, mail FROM {users} WHERE uid > 0 LIMIT 2"' => bt('Show the name and email for the first 2 users.'),
      ),
    ),
    'utf8mb4-convert' => array(
      'description' => 'Convert the database and tables to utf8mb4 encoding.',
      'callback' => 'utf8mb4_convert_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => TRUE,
      ),
      'group' => 'database',
      'aliases' => array('utf8mb4'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee utf8mb4-convert' => 'Convert the main database to utf8mb4 encoding. You will then be prompted to confirm.',
        'bee --yes utf8mb4-convert' => 'Convert the main database to utf8mb4 encoding. You will NOT be prompted to confirm.',
      ),
    ),
  );
}

/**
 * Command callback: Export the database as a compressed SQL file.
 *
 * Now includes '--no-tablespaces' by default. Many hosts need this for
 * mysqldump to work as the PROCESS permission can't be granted to the Backdrop
 * database user.
 * @see https://anothercoffee.net/how-to-fix-the-mysqldump-access-denied-process-privilege-error/
 */
function db_export_bee_callback($arguments, $options) {
  global $_bee_backdrop_root;

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

  // Get the filename for the export.
  $filename = isset($arguments['file']) ? $arguments['file'] : date('Ymd_His') . '.sql';

  // Check if any extra mysqldump options have been entered.
  if (!empty($options['extra'])) {
    // Get extra mysqldump options. If user has added the extra option, then we
    // don't include '--no-tablespaces' by default to allow the user to
    // override this setting through ommission.
    $extra = $options['extra'] . ' ';
  }
  else {
    // Include '--no-tablespaces' by default.
    $extra = '--no-tablespaces ';
  }

  $export_command = bee_get_executable_path('mysqldump');
  if (!$export_command) {
      bee_message(bt("The MySQL export command 'mysqldump' cannot be found in this system. Please install it and try again."), 'error');
    return FALSE;
  }
  // Export and compress the database.
  $export_command .= ' ';
  $export_command .= $connection_string;
  $export_command .= ' ';
  $export_command .= $extra;
  $export_command .= ' | gzip > ';
  $export_command .= $filename . '.gz';
  exec($export_command, $output, $result);
  // Remove the temporary mysql options file.
  bee_delete($connection_file);

  // Create debug message with the compiled export command.
  bee_message(bt("The compiled export command is: !export", array(
    '!export' => $export_command,
  )), 'log');

  if ($result === 0) {
    // Compile the correct filename based on whether path is absolute or
    // relative.
    if (substr($filename, 0, 1) != '/') {
      // Filename is relative path.
      $filename = $_bee_backdrop_root . '/' . $filename;
    }
    bee_message(bt("The '!database' database was exported to '!filename.gz'.", array(
      '!database' => rawurldecode($db_info['database']),
      '!filename' => $filename,
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

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

  // Get the file for the import.
  $filename = $arguments['file'];

  // Determine if this is a compressed file.
  $gzip = FALSE;
  if (substr($filename, -3) == '.gz') {
    $gzip = TRUE;
  }

  // Start preparing the import command.
  $import_command = '';

  // Define the mysql executable.
  $mysql_bin = bee_get_executable_path('mysql');
  if (!$mysql_bin) {
    bee_message(bt("The MySQL client command 'mysql' cannot be found in this system. Please install it and try again."), 'error');
    return FALSE;
  }

  // If the file is a gzip file, define the gunzip executable and add the
  // command at the start.
  if ($gzip) {
    $gunzip_bin = bee_get_executable_path('gunzip');
    if (!$gunzip_bin) {
      bee_message(bt("The gzip decompressor command 'gunzip' cannot be found in this system. Please install it and try again."), 'error');
      return FALSE;
    }
    $import_command .= "$gunzip_bin -c $filename | ";
  }

  // Check if MariaDB compatibility option is used and add into the command if
  // so.
  $mariadb_compatibility = (!empty($options['mariadb-compatibility'])) ? TRUE : FALSE;
  if ($mariadb_compatibility) {
    // Define the grep executable.
    $grep_bin = bee_get_executable_path('grep');

    // Define the grep command.
    $mariadb_compatibility_command = $grep_bin . ' -vF \'/*!999999\- enable the sandbox mode */ \'';

    if ($gzip) {
      // If the sql file is zipped then we execute the command on the output of
      // the unzipped file and pipe to mysql.
      $import_command .= "$mariadb_compatibility_command | ";
    }
    else {
      // If the sql file is not zipped then we execute the command on the file
      // and pipe to mysql.
      $import_command .= "$mariadb_compatibility_command $filename | ";
    }
  }

  // Add the mysql command with connection string.
  $import_command .= $mysql_bin . ' ' . $connection_string;
  if (!$gzip) {
    if (!$mariadb_compatibility) {
      // If the MariaDB compatibility fix is not used, then add filename to the
      // end.
      $import_command .= " < $filename";
    }
  }

  // Allow errors from mysql to go to stdout so we can store in a variable.
  $import_command .= " 2>&1";

  // Output the compiled import command as a debug message.
  bee_instant_message('$import_command: ' . $import_command, 'debug');

  // Execute the command.
  exec($import_command, $output, $result);

  // Output the result code and any output as a debug message.
  bee_instant_message('Result: ' . $result, 'debug', $output);

  // Remove the temporary mysql options file.
  bee_delete($connection_file);

  if ($result === 0) {
    bee_message(bt("'!filename' was imported into the '!database' database.", array(
      '!filename' => $filename,
      '!database' => rawurldecode($db_info['database']),
    )), 'success');
  }
  else {
    bee_message(bt('There was an error and the import could not be completed. Check the file is valid.'), 'error');
    if (strpos($output[0], 'ERROR at line 1: Unknown command \'\-\'') === 0) {
      bee_message(bt('If the source database was MariaDB, try the "--mariadb-compatibility" option.'), 'info');
    }
  }
}

/**
 * Command callback: Drop and re-create the existing configured database.
 */
function db_drop_bee_callback($arguments, $options) {
  global $_bee_yes_mode;

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];
  $db_database = rawurldecode($db_info['database']);

  if (!$_bee_yes_mode) {
    // Prompt to continue.
    if (!bee_confirm(bt('Are you sure you want to drop the contents of !database ?', array(
      '!database' => $db_database,
    )), FALSE)) {
      return;
    }
  }

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info, FALSE);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

  // Drop the existing backdrop database as configured.
  $mysql_command = bee_get_executable_path('mysql');
  if (!$mysql_command) {
    bee_message(bt("The MySQL client command 'mysql' cannot be found in this system. Please install it and try again."), 'error');
    return FALSE;
  }
  $command = $mysql_command . ' ' . $connection_string . ' -e "drop database ' . $db_database . '";';
  $result = proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));

  if ($result == -1) {
    bee_message(bt("The '!database' database could not be dropped.", array(
      '!database' => $db_database,
    )), 'error');
  }
  else {
    bee_message(bt("The '!database' database was successfully dropped.", array(
      '!database' => $db_database,
    )), 'success');
  }

  // Re-create the existing backdrop database as configured.
  $command = $mysql_command .' ' . $connection_string . ' -e "create database ' . $db_database . '";';
  $result = proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));

  if ($result == -1) {
    bee_message(bt("The '!database' database could not be created.", array(
      '!database' => $db_database,
    )), 'error');
  }
  else {
    bee_message(bt("The '!database' database was successfully created.", array(
      '!database' => $db_database,
    )), 'success');
  }
  // Remove the temporary mysql options file.
  bee_delete($connection_file);
}

/**
 * Command callback: Open an SQL command-line interface using Backdrop's
 * database credentials.
 */
function sql_bee_callback($arguments, $options) {
  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

  // Open SQL command-line.
  $mysql_command = bee_get_executable_path('mysql');
  if (!$mysql_command) {
    bee_message(bt("The MySQL client command 'mysql' cannot be found in this system. Please install it and try again."), 'error');
    return FALSE;
  }
  $command = $mysql_command . ' ' . $connection_string;
  proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));
  // Remove the temporary mysql options file.
  bee_delete($connection_file);
}

/**
 * Puts the decoded connection options into a temporary option file and returns
 * an options string pointing to the file for use with MySQL commands and the
 * filename separately, so it can be deleted after use.
 *
 * @param array $db_info
 *   Connection information for the current database. Generally obtained via
 *   Database::getConnectionInfo().
 * @param boolean $include_db
 *   Whether or not the database name is included in the connection string. By
 *   default, this is included.
 *
 * @return array
 *   An array of the option string that can be appended to 'mysql' or
 *   'mysqldump', together with the filename of the temporary options file.
 */
function db_bee_mysql_options(array $db_info, $include_db = TRUE) {
  // Prepare the credentials for the .cnf file.
  $temp_file_contents = "[client]" . PHP_EOL
    . "user=" . rawurldecode($db_info['username']) . PHP_EOL
    . "password='" . rawurldecode($db_info['password']) . "'" . PHP_EOL
    . "host=" . $db_info['host'] . PHP_EOL;

  if (isset($db_info['port']) && is_numeric($db_info['port'])) {
    $temp_file_contents .= "port=" . $db_info['port'] . PHP_EOL;
  }

  // Get the temporary directory.
  $temp_dir = bee_get_temp();
  // Create a temporary file for the username, password and host.
  $temp_filename = $temp_dir . '/db_bee.cnf';
  $temp_file = fopen($temp_filename, 'a');
  fwrite($temp_file, $temp_file_contents);
  fclose($temp_file);

  // Create an array for the connection string and filename.
  $options = array();
  $options['connection'] = "--defaults-file=$temp_filename ";
  $options['connection'] .= ($include_db) ? rawurldecode($db_info['database']) : '';
  $options['filename'] = $temp_filename;

  return $options;
}

/**
 * Command callback: Pass to db_query() after bootstrapping Backdrop.
 *
 * @see sql.drush.inc
 */
function dbq_bee_callback($arguments, $options) {
  try {
    $result = db_query($arguments['query'], array());
    $rows = $result->fetchAll(PDO::FETCH_ASSOC);
    $output = array();
    if (!empty($rows[0])) {
      // Display the output as CSV rows.
      foreach ($rows as $row) {
        if (!empty($row)) {
          $f = fopen('php://memory', 'r+');
          if (fputcsv($f, $row) === FALSE) {
              continue;
          }
          rewind($f);
          $csv_line = stream_get_contents($f);
          $text = rtrim($csv_line);
          $output[] = array(
            'type' => 'text',
            'variables' => array(
              'value' => $text,
            ),
            'newline' => TRUE,
          );
        }
      }
    }
    return $output;
  }
  catch (PDOException $e) {
    bee_message($e->getMessage(), 'error');
  }
}

/**
 * Command callback: utf-8 mb4 conversion.
 */
function utf8mb4_convert_bee_callback($arguments, $options) {
  global $_bee_yes_mode;

  $connection = Database::getConnection();
  $messages = array();
  $utf8mb4_active = $connection->utf8mb4IsActive();
  $utf8mb4_supported = $connection->utf8mb4IsSupported();
  $utf8mb4_tables_converted = state_get('database_utf8mb4_active', FALSE);

  if (!$utf8mb4_active || !$utf8mb4_supported) {
    bee_message('utf8mb4 not supported.', 'error');
    return;
  }

  // Already active and supported. Indicate completion.
  if ($utf8mb4_active && $utf8mb4_supported && $utf8mb4_tables_converted) {
    bee_message('utf8mb4 is already enabled on your site, no further action is needed.');
    return;
  }

  global $databases;
  $database = $databases['default']['default'];

  if (!$_bee_yes_mode) {
    // Prompt to continue.
    if (!bee_confirm(bt('The upgrade process is irreversible and in the event of an error, you will want to restore a previous version. You are recommended to create a database backup before proceeding; you can do this with the "db-export" command if you have not already created one. Do you want to proceed with the conversion of tables in the "!database" database now?', array(
      '!database' => $database['database'],
    )), FALSE)) {
      return;
    }
  }

  // Enter maintenance mode.
  state_set('maintenance_mode', TRUE);

  // Build the list of tables to convert.
  $like = '';
  if (!empty($database['prefix'])) {
    $like = ' LIKE "' . $database['prefix'] . '%"';
  }
  $table_names = db_query('SHOW TABLES' . $like)->fetchCol();

  // Instantiate a character set converter object.
  $converter = new DatabaseCharsetConverter();

  try {
    $result = $converter->convertDatabase($database['database']);
    if (!$result) {
      $messages[] = t('The database %name could not be converted.', array('%name' => $database['database']));
      bee_message(implode(', ', $messages), 'error');
      return;
    }
  }
  catch (PDOException $e) {
    $messages[] = $e->getMessage();
    bee_message(implode(', ', $messages), 'error');
    return;
  }

  foreach ($table_names as $table_name) {
    try {
      $result = $converter->convertTable($table_name);
      if (!$result) {
        $messages[] = t('The table %name could not be converted.', array('%name' => $table_name));
      }
    }
    catch (PDOException $e) {
      $messages[] = $e->getMessage();
    }
  }
  bee_message(implode(', ', $messages));
  state_set('maintenance_mode', FALSE);
  state_set('database_utf8mb4_active', TRUE);
  bee_message('Database and tables successfully converted.');
}
