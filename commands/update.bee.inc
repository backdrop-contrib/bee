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

    // Take the site offline while schema changes run, as update_batch() does.
    $_SESSION['maintenance_mode'] = state_get('maintenance_mode', FALSE);
    if ($_SESSION['maintenance_mode'] == FALSE) {
      state_set('maintenance_mode', TRUE);
    }

    // Resolve dependencies to order the updates.
    $updates = update_resolve_dependencies($start);
    $dependency_map = array();
    foreach ($updates as $function => $update) {
      $dependency_map[$function] = !empty($update['reverse_paths']) ? array_keys($update['reverse_paths']) : array();
    }

    // Run each update directly (not via update_batch) so its result prints as it finishes.
    $context = array('results' => array());
    foreach ($updates as $update) {
      if (empty($update['allowed'])) {
        continue;
      }
      $module = $update['module'];
      $number = $update['number'];
      $function = $module . '_update_' . $number;

      // Start each module just before its first pending update.
      if (isset($start[$module])) {
        backdrop_set_installed_schema_version($module, $number - 1);
        unset($start[$module]);
      }

      // bee_instant_message() prints now; bee_message() would batch to the end.
      bee_instant_message(bt('Update started: !function', array('!function' => $function)), 'info');

      // Long-running updates re-run until $context['finished'] reaches 1.
      $context['sandbox'] = array();
      do {
        $context['finished'] = 1;
        update_do_one($module, $number, $dependency_map[$function], $context);
      } while ($context['finished'] < 1);

      $result = isset($context['results'][$module][$number]) ? $context['results'][$module][$number] : array();
      if (!empty($result['results']['query'])) {
        bee_instant_message(trim(strip_tags($result['results']['query'])), !empty($result['results']['success']) ? 'success' : 'error');
      }

      // Stop on abort; dependent updates can no longer run safely.
      if (!empty($context['results']['#abort'])) {
        if (!empty($result['#abort']['query'])) {
          bee_instant_message(trim(strip_tags($result['#abort']['query'])), 'error');
        }
        bee_instant_message(bt('Update aborted: !function', array('!function' => $function)), 'error');
        break;
      }

      bee_instant_message(bt('Update completed: !function', array('!function' => $function)), 'info');
    }

    // Flush caches and restore the site, as update_finished() does.
    backdrop_flush_all_caches();
    if (isset($_SESSION['maintenance_mode']) && $_SESSION['maintenance_mode'] == FALSE) {
      state_set('maintenance_mode', FALSE);
      unset($_SESSION['maintenance_mode']);
    }

    bee_instant_message(bt('Finished performing updates.'), 'success');
  }
}
