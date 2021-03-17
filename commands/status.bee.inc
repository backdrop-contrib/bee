<?php
/**
 * @file
 * Command(s) for getting an overview of a Backdrop installation.
 */

/**
 * Implements hook_bee_command().
 */
function status_bee_command() {
  return array(
    'status' => array(
      'description' => bt('Provides an overview of the current Backdrop installation/site.'),
      'callback' => 'status_bee_callback',
      'options' => array(
        'show-password' => array(
          'description' => bt('Show the database password.'),
        ),
      ),
      'aliases' => array('st', 'info', 'core-status'),
      'examples' => array(
        'bee status' => bt('Get an overview of the Backdrop installation.'),
        'bee status --show-password' => bt('Include the database password in the overview.'),
      ),
    ),
  );
}

/**
 * Command callback: Provides an overview of the current Backdrop
 * installation/site.
 */
function status_bee_callback($arguments, $options) {
  global $_bee_backdrop_root, $_bee_backdrop_site, $_bee_multisites;
  $rows = array();

  if (!empty($_bee_backdrop_root)) {
    // Backdrop core.
    if (defined('BACKDROP_VERSION')) {
      $rows[] = array(
        array('value' => bt('Backdrop CMS')),
        array('value' => BACKDROP_VERSION),
      );
    }
    $rows[] = array(
      array('value' => bt('Root directory')),
      array('value' => $_bee_backdrop_root),
    );

    // Multisite.
    $rows[] = array(
      array('value' => bt('Site type')),
      array('value' => empty($_bee_multisites) ? bt('Single') : bt('Multisite')),
    );
    if (!empty($_bee_backdrop_site)) {
      $rows[] = array(
        array('value' => bt('Site directory')),
        array('value' => $_bee_backdrop_site),
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
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}
