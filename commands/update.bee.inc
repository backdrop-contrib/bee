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
      'optional_arguments' => array('project'),
      'multiple_argument' => 'project',
      'options' => array(
        'security-updates-only' => array(
          'description' => bt('Only updates projects/backdrop with security releases'),
        ),
      ),
      'examples' => array(
        'bee update' => bt('Update everything with a new release.'),
        'bee update webform tatsu' => bt('Updates the Webform module and Tatsu theme only'),
        'bee update backdrop --security-updates-only' => bt('Updates only core backdrop and only security releases'),
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
          '!module' => $module
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
 * Command callback: Show, and optionally apply, all core and module updates.
 */
function update_bee_callback() {
/*
  * @todo consider if any drush extension code can be re-used https://github.com/backdrop-contrib/backdrop-drush-extension/blob/1.x-1.x/commands/pm/backdrop_pm.drush.inc
  * @todo consider if brush can be re-used https://github.com/backdrop-contrib/brush/blob/1.x-1.x/commands/pm/updatecode.pm.inc
  * @todo check if file permissions allow backdrop to update itself? 
  * @todo determine required files:
  *   - core/modules/update/update.fetch.inc
  *   - core/modules/system/system.updater.inc ?? (https://docs.backdropcms.org/api/backdrop/core%21modules%21system%21system.updater.inc/1)
  *   - core/includes/update.inc
  *   - core/modules/installer/installer.manager.inc (https://docs.backdropcms.org/api/backdrop/core%21modules%21installer%21installer.manager.inc/1)
  *   - core/modules/installer/installer.manager.inc (https://docs.backdropcms.org/api/backdrop/core%21modules%21installer%21installer.manager.inc/function/installer_manager_update_ready_form/1)
  * @todo process arguments (list of projects)
  * @todo process option (security-updates-only)
  * @todo check for config item - update_disabled_extensions (Configuration>Update settings>) - if this is true (1) then need to check disabled projects also
  * @todo get list of installed projects (core, modules, themes, layouts) if no argument list of projects
  * @todo list of projects is either all installed + core, all enabled + core or argument list of projects
  * @todo get current backdrop version
  * @todo for each project in list get current version, type, name
  * @todo for each project in list get available version
  * @todo compare versions and prepare list of possible updates
  * @todo ??option to review and proceed or cancel??
  * @todo download updates to temp directory
  * @todo install updates
  * @todo can we / should we check if database updates needed and let user know
*/
}