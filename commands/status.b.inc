<?php
/**
 * @file
 * Command(s) for getting an overview of a Backdrop installation.
 */

/**
 * Implements hook_b_command().
 */
function status_b_command() {
  return array(
    'status' => array(
      'description' => bt('Provides an overview of the current Backdrop installation/site.'),
      'callback' => 'status_b_callback',
      'options' => array(
        'show-password' => array(
          'description' => bt('Show the database password.'),
        ),
      ),
      'aliases' => array('st', 'core-status'),
      'examples' => array(
        'b status' => bt('Get an overview of the Backdrop installation.'),
        'b status --show-password' => bt('Include the database password in the overview.'),
      ),
    ),
  );
}

/**
 * Command callback: Provides an overview of the current Backdrop
 * installation/site.
 */
function status_b_callback($arguments, $options) {
  global $backdrop_root, $backdrop_site, $multisites;
  $rows = array();

  // Backdrop core.
  if (defined('BACKDROP_VERSION')) {
    $rows[] = array(
      array('value' => bt('Backdrop CMS')),
      array('value' => BACKDROP_VERSION),
    );
  }
  if (!empty($backdrop_root)) {
    $rows[] = array(
      array('value' => bt('Root directory')),
      array('value' => $backdrop_root),
    );
  }

  // Multisite.
  $rows[] = array(
    array('value' => bt('Site type')),
    array('value' => empty($multisites) ? bt('Single') : bt('Multisite')),
  );
  if (!empty($backdrop_site)) {
    $rows[] = array(
      array('value' => bt('Site directory')),
      array('value' => $backdrop_site),
    );
  }

  // Database.
  if (class_exists('Database')) {
    $db_connection = Database::getConnectionInfo();
    if (!empty($info = $db_connection['default'])) {
      $rows[] = array(
        array('value' => bt('Database')),
        array('value' => $info['driver']),
      );
      $rows[] = array(
        array('value' => bt('Database name')),
        array('value' => $info['database']),
      );
      $rows[] = array(
        array('value' => bt('Database username')),
        array('value' => $info['username']),
      );
      $rows[] = array(
        array('value' => bt('Database password')),
        array('value' => isset($options['show-password']) ? $info['password'] : '**********'),
      );
      $rows[] = array(
        array('value' => bt('Database host')),
        array('value' => $info['host']),
      );
      $rows[] = array(
        array('value' => bt('Database port')),
        array('value' => $info['port']),
      );
    }
  }

  echo "\n";
  return array(
    array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'delimiter' => '   ',
        'delimiter_left' => ' ',
        'delimiter_right' => ' ',
      ),
    ),
  );
}
