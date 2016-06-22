<?php
/**
 * Implements of hook_b_command().
 */
function updatedb_b_command() {
  $items['updatedb-status'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => bt('Show a list of updates.'),
    'callback' => 'updatedb_status_callback',
    'aliases' => array('updbst'),
  );
  $items['updatedb'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => bt('Apply any database updates required (as with running update.php).'),
    'callback' => 'updatedb_callback',
    'aliases' => array('updb'),
  );

  return $items;
}

/**
  * Callback function for hook_b_command().
  *   respond on command 'updatedb-status'
  *
  * @param array $arguments
  * @param array $options
  * @return array $output
  *  Table of pending updates.
  */
function updatedb_status_callback($arguments, $options){
  require_once 'core/includes/install.inc';
  backdrop_load_updates();
  require_once 'core/includes/update.inc';
  $pending = update_get_update_list();

  if (empty($pending)) {
    b_set_message(bt("No database updates required"));
    return;
  }

  list($update, $start) = updatedb_status();
  return update_table($update);
}

/**
  * Callback function for hook_b_command().
  *   respond on command 'updatedb'
  *
  * @param array $arguments
  * @param array $options
  * @return array $output
  *  Table of pending updates.
  */
function updatedb_callback($arguments, $options){

  require_once 'core/includes/install.inc';
  backdrop_load_updates();
  require_once 'core/includes/update.inc';
  require_once 'core/includes/batch.inc';
  $pending = update_get_update_list();

  if (empty($pending)) {
    b_set_message(bt("No database updates required"));
    return;
  }

  list($update, $start) = updatedb_status();

  b_render(update_table($update));

  if (!b_confirm(bt('Do you wish to run all pending updates?'))) {
    b_set_message(bt('Cancelled'));
    return;
  }

  // Make batch run right away, without background processing
  $batch = &batch_get();
  $batch['progressive'] = 0;
  update_batch($start, NULL, NULL, array('progressive' => 0));
  b_set_message(bt('All updates processed'), 'success');
}

/**
 * Function to prepare pending updates table.
 *
 * @param array $update
 * @return array $output
 *  Table of pending updates.
 */
function update_table($update){
  $rows = array();
  $modules = array();
  foreach ($update as $line) {
    $modules[$line['module']][] = $line;
  }

  $output = array();
  foreach ($modules as $module => $updates) {
    $output[] = array(
      '#type' => 'text',
      'value' => bt('Module !name', array('!name' => $module)),
      '#color' => 'bold_blue',
    );
    $rows = array();
    foreach ($updates as $update) {
      $rows[] = array(
        '',
        array('data' => $update['update_id'], '#color' => 'purple'),
        $update['description'],
      );
    }

    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => '  ',
      'delimiter_left' => ' ',
      'delimiter_right' => ' ',
    );

    $output[] = array(
      '#type' => 'text',
      'value' => "",
    );
  }
  return $output;
}

/**
 * Return a 2 item array with
 *  - an array where each item is a 3 item associative array describing a pending update.
 *  - an array listing the first update to run, keyed by module.
 */
function updatedb_status() {
  $pending = update_get_update_list();

  $return = array();
  // Ensure system module's updates run first.
  $start['system'] = array();

  // Print a list of pending updates for this module and get confirmation.
  foreach ($pending as $module => $updates) {
    if (isset($updates['start']))  {
      foreach ($updates['pending'] as $update_id => $description) {
        // Strip cruft from front.
        $description = str_replace($update_id . ' - ', '', $description);
        $return[] = array('module' => ucfirst($module), 'update_id' => $update_id, 'description' => $description);
      }
      if (isset($updates['start'])) {
        $start[$module] = $updates['start'];
      }
    }
  }
  return array($return, $start);
}
