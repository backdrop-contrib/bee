<?php
/**
 * @file
 * Command(s) for updating Backdrop projects.
 */

/**
 * Implements hook_b_command().
 */
function update_b_command() {
  return array(
    'update-db' => array(
      'description' => bt('Show, and optionally apply, all pending database updates.'),
      'callback' => 'update_db_b_callback',
      'aliases' => array('updb', 'updbst', 'updatedb', 'updatedb-status'),
      'bootstrap' => B_BOOTSTRAP_FULL,
      'examples' => array(
        'b update-db' => bt('Show a list of any pending database updates. You will then be prompted to apply them.'),
      ),
    ),
  );
}

/**
 * Command callback: Show, and optionally apply, all pending database updates.
 */
function update_db_b_callback() {
  global $backdrop_root;
  $start = array();
  $modules = array();

  require_once $backdrop_root . '/core/includes/install.inc';
  require_once $backdrop_root . '/core/includes/update.inc';

  // Get list of pending database updates.
  backdrop_load_updates();
  $pending = update_get_update_list();
  if (empty($pending)) {
    b_message(bt('There are no pending database updates.'), 'info');
    return;
  }

  // Process the list of updates.
  foreach ($pending as $module => $updates) {
    if (!isset($updates['start'])) {
      // Display warning about updates that cannot be run.
      if (!empty($updates['warning'])) {
        b_message($updates['warning'], 'warning');
      }
      else {
        b_message(bt("'@module' module can not be updated due to unresolved requirements."), 'warning');
      }
      continue;
    }
    else {
      $start[$module] = $updates['start'];
    }

    foreach ($updates['pending'] as $update_id => $description) {
      $description = str_replace("$update_id - ", '', $description);
      $modules[$module][] = array(
        'update_id' => $update_id,
        'description' => $description,
      );
    }
  }

  // Display pending updates.
  echo "\n";
  foreach ($modules as $module => $updates) {
    b_render_text(array(
      'value' => bt('@module module', array('@module' => $module)),
      '#color' => 'blue',
      '#bold' => TRUE,
    ));

    $rows = array();
    foreach ($updates as $update) {
      $rows[] = array(
        array('value' => $update['update_id']),
        array('value' => $update['description']),
      );
    }
    b_render_table(array('rows' => $rows));
  }

  // Prompt to apply updates.
  if (!b_confirm(bt('Would you like to apply all pending updates?'))) {
    return;
  }

  // Apply the updates, without calling `backdrop_goto()`.
  // @see batch_process()
  $batch = &batch_get();
  $batch['progressive'] = FALSE;
  update_batch($start);
  b_message(bt('All pending updates applied.'), 'success');
}
