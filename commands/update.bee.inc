<?php
/**
 * @file
 * Command(s) for updating Backdrop projects.
 */

/**
 * Implements hook_bee_command().
 */
function update_bee_command()
{
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
function update_db_bee_callback()
{
    global $_bee_backdrop_root;
    $start = array();
    $modules = array();

    require_once $_bee_backdrop_root . '/core/includes/install.inc';
    require_once $_bee_backdrop_root . '/core/includes/update.inc';

    // Get list of pending database updates.
    backdrop_load_updates();
    $pending = update_get_update_list();
    if (empty($pending)) {
        bee_message(bt('There are no pending database updates.'));
        return;
    }

    // Process the list of updates.
    foreach ($pending as $module => $updates) {
        if (!isset($updates['start'])) {
            // Display warning about updates that cannot be run.
            if (!empty($updates['warning'])) {
                bee_message($updates['warning'], 'warning');
            } else {
                bee_message(bt("'!module' module can not be updated due to unresolved requirements.", array(
                    '!module' => $module,
                )), 'warning');
            }
            continue;
        } else {
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
 * 2023-03-06 a first attempt at getting this working
 */
function update_bee_callback()
{
    global $_bee_backdrop_root;

    require_once $_bee_backdrop_root . '/core/includes/file.inc';

    $themes = list_themes();
    $data = null;
    if ($available = update_get_available(TRUE)) {
        module_load_include('inc', 'update', 'update.compare');
        $data = update_calculate_project_data($available);
    }

    if ($data != null) {
        foreach ($data as $item) {
            if ($item['name'] == 'backdrop') {
                unset($data['backdrop']);
            } else
                if ($item['existing_version'] == $item['latest_version']) {
                    unset($data[$item['name']]);
                }
            }
        }
        if ($data != null) {
            bee_message(bt('These are the items being updated.....'));
            foreach ($data as $item) {
                bee_message(bt($item['name'] . ' upgrading from ' . $item['existing_version'] .
                    ' to ' . $item['latest_version']));
            }
            // TODO Prompt to continue. but it does not show
            // if (!bee_confirm(bt('Would you like to continue?'), FALSE)) {
            //    return;
            // }
            foreach ($data as $item) {
                $folder = $_bee_backdrop_root . '/' . $item['project_type'] . 's/' . $item['name'];
                if (is_dir($folder)) {
                    delete_dir($folder);
                }
                download_bee_callback(array('projects' => array($item['name']),), []);
                echo '';
            }
            bee_message(bt('You will probably need to fix the permissions on the new modules'), 'warning');
        } else {
            bee_message(bt('No Modules or Themes to Update'));
        }
    } else {
        bee_message(bt('No Modules or Themes to Update'));
    }

}

/*
 * Delete a folder recursively
 * checks for root folder and exits
 * */
/**
 * @param $path - Path of folder to delete
 * @return bool
 */
function delete_dir($path): bool
{
    if (empty($path)) {
        return false;
    }
    $files = array_diff(scandir($path), array('.', '..'));
    foreach ($files as $file) {
        (is_dir("$path/$file")) ? delete_dir("$path/$file") : unlink("$path/$file");
    }
    return rmdir($path);
}
