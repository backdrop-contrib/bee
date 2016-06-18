<?php

/**
 * Implementation of hook_b_command().
 */
function status_b_command() {
  $items = array();
  // We specify command callbacks here because the defaults would collide with
  // the drush cache api functions.
 $items['core-status'] = array(
    'description' => 'Provides a birds-eye view of the current Backdrop installation, if any.',
    'bootstrap' => B_BOOTSTRAP_FULL,
    'aliases' => array('status', 'st'),
    'examples' => array(
      'b core-status ' => 'Show status.',
      'b core-status --show-passwords' => 'Show status include password.',
    ),
    'options' => array(
      'show-passwords' => 'Show database password.',
    ),
    'callback' => 'status_b_callback',
  );

  return $items;
}

function status_b_callback($arguments, $options) {
  $rows = array();
  if(defined('BACKDROP_VERSION')){
    $color = 'green';
    $rows[] = array('Backdrop CMS', array('data' => BACKDROP_VERSION, '#color' => $color));
  }

  if(defined('BACKDROP_ROOT')){
    $rows[] = array('Backdrop root', BACKDROP_ROOT);
  }
  if(class_exists('Database')){
    if($connection = Database::getConnection('default', 'default')) {
      $conn = $connection->getConnectionOptions();
      $rows[] = array('Database driver', $conn['driver']);
      $rows[] = array('Database host', $conn['host']);

      $rows[] = array('Database name', $conn['database']);
      $rows[] = array('Database username', $conn['username']);
      if(isset($options['show-passwords']) or isset($options['full'])) {
        $rows[] = array('Database password', array('data' => $conn['password'], '#color' => 'red'));
      }
    }
  }
  else{
    b_set_message('No database connection', 'warning');
  }

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '  :  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}
