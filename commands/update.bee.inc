<?php
/**
 * @file
 * Command(s) for updating Backdrop projects.
 */

/**
 * Implements hook_bee_command().
 */
function update_bee_command() {
  return array(
    'update-db' => array(
      'description' => bt('Show, and optionally apply, all pending database updates.'),
      'callback' => 'update_db_bee_callback',
      'group' => 'update',
      'aliases' => array('updb', 'updbst', 'updatedb', 'updatedb-status'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee update-db' => bt('Show a list of any pending database updates. You will then be prompted to apply them.'),
      ),
    ),
  );
}

/**
 * Command callback: Show, and optionally apply, all pending database updates.
 */
function update_db_bee_callback() {
  global $_bee_backdrop_root;
  $start = array();
  $modules = array();

  require_once $_bee_backdrop_root . '/core/includes/install.inc';
  require_once $_bee_backdrop_root . '/core/includes/update.inc';

  // Get list of pending database updates.
  backdrop_load_updates();
  $pending = update_get_update_list();
  if (empty($pending)) {
    bee_message(bt('There are no pending database updates.'), 'info');
    return;
  }

  // Process the list of updates.
  foreach ($pending as $module => $updates) {
    if (!isset($updates['start'])) {
      // Display warning about updates that cannot be run.
      if (!empty($updates['warning'])) {
        bee_message($updates['warning'], 'warning');
      }
      else {
        bee_message(bt("'!module' module can not be updated due to unresolved requirements.", array(
          '!module' => $module,
        )), 'warning');
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
  if (!empty($modules)) {
    echo "\n";
    foreach ($modules as $module => $updates) {
      bee_render_text(array(
        'value' => bt('!module module', array('!module' => $module)),
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
      bee_render_table(array('rows' => $rows));
    }

    // Prompt to apply updates.
    if (!bee_confirm(bt('Would you like to apply all pending updates?'))) {
      return;
    }

    // Apply the updates, without calling `backdrop_goto()`.
    // @see batch_process()
    $batch = &batch_get();
    $batch['progressive'] = FALSE;
    update_batch($start);
    bee_message(bt('All pending updates applied.'), 'success');
  }
}
