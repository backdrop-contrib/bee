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
    'update' => array(
      'description' => bt('Updates backdrop, modules, themes and layouts with new releases'),
      'callback' => 'update_bee_callback',
      'aliases' => array('up', 'pm-update'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'arguments' => array(
        'projects' => bt('One or more projects to update.'),
      ),
      'optional_arguments' => array('projects'),
      'multiple_argument' => 'projects',
      'options' => array(
        'security-only' => array(
          'description' => bt('Only update modules that have security updates available.'),
          'short' => 's',
        ),
      ),
      'examples' => array(
        'bee update' => bt('Update everything with a new release.'),
        'bee update --security-only' => bt('Update everything that is a security update only'),
        'bee update webform tatsu' => bt('Updates the Webform module and Tatsu theme only'),
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

/**
 * Command callback: Show, and apply, module and theme updates.
 *
 * @todo  Add core update.
 * @todo  Consider a way to download first, check the download has worked before delete.
 *
 */
function update_bee_callback($arguments, $options) {
  global $_bee_backdrop_root;
  require_once $_bee_backdrop_root . '/core/includes/file.inc';
  $data = NULL;
  if ($available = update_get_available(TRUE)) {
    module_load_include('inc', 'update', 'update.compare');
    $data = update_calculate_project_data($available);
  }
  if ($data != NULL) {
    foreach ($data as $item) {
      if ($item['name'] == 'backdrop') {
        unset($data['backdrop']);
        continue;
      }
      if (!empty($arguments['projects']) && !in_array($item['name'], $arguments['projects'])) {
        unset($data[$item['name']]);
        continue;
      }
      if (!isset($item['latest_version'])) {
        unset($data[$item['name']]);
        continue;
      }
      if ($item['existing_version'] == $item['latest_version']) {
        unset($data[$item['name']]);
        continue;
      }
      if ((isset($options['security-only']) || isset($options['s'])) && $item['status'] !== UPDATE_NOT_SECURE) {
        unset($data[$item['name']]);
      }
    }
  }
  if ($data != NULL) {
    foreach ($data as $item) {
      $rows[] = array(
        array('value' => $item['name']),
        array('value' => $item['existing_version']),
        array('value' => $item['latest_version']),
      );
    }
    bee_render_text(array('value' => bt("These are the items being updated:\n")));
    bee_render_table(array(
      'rows' => $rows,
      'header' => array(
        array('value' => bt('Module')),
        array('value' => bt('Existing')),
        array('value' => bt('Latest')),
      ),
    ));
    // Prompt to continue.
    if (!bee_confirm(bt('Would you like to continue?'), FALSE)) {
      return;
    }
    foreach ($data as $item) {
      $folder = backdrop_get_path($item['project_type'], $item['name']);
      if (is_dir($folder)) {
         bee_delete($folder);
      }
      download_bee_callback(array('projects' => array($item['name']),), []);
      echo '';
    }
  } else {
    bee_message(bt('No Modules or Themes to Update'));
  }
}
