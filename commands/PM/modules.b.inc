<?php
/**
 * Implements hook_b_command().
 */
function modules_b_command() {
  $items['pm-list'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Show a list of available extensions (modules, layouts or themes).',
    'options' => array(
      'type' => 'Filter by extension type. Choices: module, theme, layout.',
    ),
    'callback' => 'modules_b_list_callback',
    'aliases' => array('pml'),
  );
  $items['pm-enable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Enable one or more modules.',
    'arguments' => array(
      'extensions' => array(
        'description' => 'List of modules.',
        'multiple' => TRUE,
      ),
    ),
    'callback' => 'modules_b_enable_callback',
    'aliases' => array('en'),
  );
  $items['pm-disable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Disable one or more modules.',
    'arguments' => array(
      'extensions' => array(
        'description' => 'List of modules.',
        'multiple' => TRUE,
      ),
    ),
    'aliases' => array('dis'),
    'callback' => 'modules_b_disable_callback',
  );
  $items['pm-uninstall'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Uninstall one or more modules.',
    'arguments' => array(
      'modules' => array(
        'description' => 'List of modules.',
        'multiple' => TRUE,
      ),
    ),
    'aliases' => array('pmu'),
    'callback' => 'modules_b_uninstall_callback',
  );
  $items['pm-info'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Show detailed info for one or more extensions (modules or themes).',
    'arguments' => array(
      'extensions' => array(
        'description' => 'List of extensions.',
        'multiple' => TRUE,
      ),
    ),
    'aliases' => array('pmi'),
    'callback' => 'modules_b_info_callback',
  );
  return $items;
}

/**
 * pm-list callback. Display list of modules, themes and layouts.
 *
 * @param array $arguments
 * @param array $options
 * @return array
 */
function modules_b_list_callback($arguments, $options){

  if(!isset($options['type']) or $options['type'] != 'layout') {

    //Rescan for new modules.
    system_rebuild_module_data();

    //Rescan for new themes.
    system_rebuild_theme_data();

    $query = db_select('system', 's')
      ->fields('s');

    if (isset($options['type']) && in_array($options['type'], array('module', 'theme'))) {
      $query->condition('type', $options['type']);
    }

    $projects = $query->execute()->fetchAll();

    $rows = array();

    foreach($projects as $project) {
      $project->info = unserialize($project->info);
      if (isset($project->info['hidden']) && $project->info['hidden']) {
        continue;
      }

      $status = array(
        'data' => '  [ ]  ',
        '#color' => 'bold_gray',
      );
      if ($project->status) {
        $status = array(
          'data' => '  [v]  ',
          '#color' => 'green',
        );
      }

      $name = array(
          'data' => $project->name,
          '#color' => 'bold_blue',
        );
      $rows[] = array(
        $name,
        $project->info['name'],
        $project->type,
        $status,
        isset($project->info['version']) ? $project->info['version'] : bt('Unknown'),
      );

    }
  }

  if (!isset($options['type']) or $options['type'] == 'layout') {
    $used_layouts = layout_load_all();

    $layouts = layout_get_layout_template_info();
    foreach($layouts as $layout) {
      $name = array(
        'data' => $layout['name'],
        '#color' => 'bold_blue',
      );

      $status = FALSE;
      foreach($used_layouts as $used_layout) {
        if($used_layout->layout_template == $layout['name']) {
          $status = array(
            'data' => '  [v]  ',
            '#color' => 'green',
          );
        }
      }
      if (!$status) {
        $status = array(
          'data' => '  [ ]  ',
          '#color' => 'bold_gray',
        );
      }

      $rows[] = array(
        $name,
        $layout['title'],
        'layout',
        $status,
        $layout['version'],
      );
    }
  }
  $header = array(
    'Project',
    'Name',
    'Type',
    'Status',
    'Version',
  );

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'header' => $header,
    'delimiter' => '  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}

/**
 * pm-enable callback. Enable module.
 *
 * @param array $arguments
 * @param array $options
 *
 */
function modules_b_enable_callback($arguments, $options) {
  if (!isset($arguments['extensions'])) {
    b_set_message('Please provide module name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['extensions'] as $project ) {
    if (_enable_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if ($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to enable module or theme.
 *
 * @param string $project
 *  The project machine name to be enabled.
 * @return bool
 */
function _enable_project($project) {
  // Rescan for new modules.
  system_rebuild_module_data();

  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    return FALSE;
  }

  if ($module['status']) {
    b_set_message("Failed to enable module " . $module['name']. ": it is enabled already", 'error');
    return FALSE;
  }

  $rows = array();

  $info =  unserialize($module['info']);

  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Package'), '#color' => 'bold_blue'),
    $info['package'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to enable?'))) {
    b_set_message(bt('Cancelled to enable !name', array('!name' => $module['name'])));
    return;
  }

  if (module_enable(array($module['name']), FALSE)) {
    b_set_message("Module " . $module['name'] . " enabled", 'success');
    return TRUE;
  }
  b_set_message("Failed to enable module " . $module['name'], 'error');
  return FALSE;
}

/**
 * pm-disable callback. Disable module or theme.
 *
 * @param array $arguments
 * @param array $options
 */
function modules_b_disable_callback($arguments, $options) {

  if (!isset($arguments['extensions'])) {
    b_set_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['extensions'] as $project ) {
    if (_disable_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if ($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module or theme.
 *
 * @param sring $project
 *  Project machine name to be disabled.
 */
function _disable_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    return FALSE;
  }

  if (!$module['status']) {
    b_set_message("Failed to disable module " . $module['name'] . ": it is disabled already", 'error');
    return FALSE;
  }

  $rows = array();

  $info =  unserialize($module['info']);

  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Package'), '#color' => 'bold_blue'),
    $info['package'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to disable?'))) {
    b_set_message(bt('Cancelled to disable !name', array('!name' => $module['name'])));
    return;
  }

  module_disable(array($module['name']), FALSE);
  b_set_message("Module " . $module['name'] . "  disabled", 'success');
  return TRUE;
}

/**
 * pm-unstall callback. Uninstall module.
 *
 * @param array $arguments
 * @param array $options
 */
function modules_b_uninstall_callback($arguments, $options) {
  if (!isset($arguments['modules'])) {
    b_set_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['modules'] as $project ) {
    if (_uninstall_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if ($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module or theme.
 *
 * @param string $project
 *  The machine name of the project to be uninstalled.
 */
function _uninstall_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');

  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    b_set_message(bt("Module !name not found", array('!name' => $project)), 'error');
    return FALSE;
  }

  if ($module['status']) {
    b_set_message("Failed to uninstall module " . $module['name']. ": it is need to be disabled first", 'error');
    return FALSE;
  }

  $rows = array();

  $info =  unserialize($module['info']);

  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Package'), '#color' => 'bold_blue'),
    $info['package'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to uninstall?'))) {
    b_set_message(bt('Cancelled to enable !name', array('!name' => $module['name'])));
    return;
  }

  include_once BACKDROP_ROOT . '/core/includes/install.inc';

  if (backdrop_uninstall_modules(array($module['name']), FALSE)) {
    b_set_message("Module " . $module['name'] . " uninstalled", 'success');
    return TRUE;
  }
  else {
    b_set_message("Failed to uninstall module " . $module['name'], 'error');
  }
}

/**
 * pm-info callback. Provide info about extention.
 *
 * @param array $arguments
 * @param array $options
 */
function modules_b_info_callback($arguments, $options) {
  if (!isset($arguments['extensions'])) {
    b_set_message('Please provide project name(s)', 'error');
    return;
  }
  foreach($arguments['extensions'] as $project) {
    _info_project($project);
  }
}

/**
 * Internal function to provide detailed information about module or theme.
 *
 * @param string $project
 *  The machine name of the project about which info is required.
 */
function _info_project($project) {

  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);

  $extension = $query->execute()->fetchAssoc();
  if (!$extension) {
    b_set_message(bt("Extension !name not found", array('!name' => $project)), 'error');
  } else {
    $rows = array();

    $info =  unserialize($extension['info']);

    $rows[] = array(
      array('data' => bt('Extension'), '#color' => 'bold_blue'),
      $extension['name'],
    );
    if (!empty($info['project'])) {
      $rows[] = array(
        array('data' => bt('Project'), '#color' => 'bold_blue'),
        isset($info['project']),
      );
    }
    $rows[] = array(
      array('data' => bt('Type'), '#color' => 'bold_blue'),
      $info['type'],
    );
    $rows[] = array(
      array('data' => bt('Title'), '#color' => 'bold_blue'),
      $info['name'],
    );
    $rows[] = array(
      array('data' => bt('Description'), '#color' => 'bold_blue'),
      $info['description'],
    );
    $rows[] = array(
      array('data' => bt('Version'), '#color' => 'bold_blue'),
      $info['version'],
    );
    $rows[] = array(
      array('data' => bt('Date'), '#color' => 'bold_blue'),
      format_date($info['mtime'], 'custom', 'Y-m-d'),
    );
    if (!empty($info['package'])) {
      $rows[] = array(
        array('data' => bt('Package'), '#color' => 'bold_blue'),
        $info['package'],
      );
    }
    $rows[] = array(
      array('data' => bt('Core'), '#color' => 'bold_blue'),
      $info['backdrop'],
    );
    $rows[] = array(
      array('data' => bt('PHP'), '#color' => 'bold_blue'),
      $info['php'],
    );
    $rows[] = array(
      array('data' => bt('Status'), '#color' => 'bold_blue'),
      $extension['status'] == '1' ? 'enabled' : 'disabled',
    );
    $rows[] = array(
      array('data' => bt('Path'), '#color' => 'bold_blue'),
      substr($extension['filename'], 0, strrpos($extension['filename'], '/')),
    );
    if (!empty($extension['schema_version'])) {
      $rows[] = array(
        array('data' => bt('Schema version'), '#color' => 'bold_blue'),
        $extension['schema_version'],
      );
    }
    $rows[] = array(
      array('data' => bt('Files'), '#color' => 'bold_blue'),
      !empty($info['files']) ? implode(', ', $info['files']) : 'none',
    );
    $rows[] = array(
      array('data' => bt('Requires'), '#color' => 'bold_blue'),
      !empty($info['dependencies']) ? implode(', ', $info['dependencies']) : 'none',
    );
    $rows[] = array(
      array('data' => bt('Configure'), '#color' => 'bold_blue'),
      isset($info['configure']) ? $info['configure'] : 'none',
    );
    if (!empty($info['tags'])) {
      $rows[] = array(
        array('data' => bt('Tags'), '#color' => 'bold_blue'),
        !empty($info['tags']) ? implode(', ', $info['tags']) : 'none',
      );
    }
    if (!empty($info['engine'])) {
      $rows[] = array(
        array('data' => bt('Engine'), '#color' => 'bold_blue'),
        $info['engine'],
      );
    }
    if (!empty($info['screenshot'])) {
      $rows[] = array(
        array('data' => bt('Screenshot'), '#color' => 'bold_blue'),
        $info['screenshot'],
      );
    }
    if (!empty($info['stylesheets'])) {
      foreach ($info['stylesheets'] as $key => $value) {
        foreach ($value as $key => $value) {
          $stylesheets[] = $value;
        }
      }
      $rows[] = array(
        array('data' => bt('Stylesheets'), '#color' => 'bold_blue'),
        implode(', ', $stylesheets),
      );
    }
    if (!empty($info['ckeditor_stylesheets'])) {
      $rows[] = array(
        array('data' => bt('Ckeditor stylesheets'), '#color' => 'bold_blue'),
        $info['ckeditor_stylesheets'] ? implode(', ', $info['ckeditor_stylesheets']) : 'none',
      );
    }
    if (!empty($info['scripts'])) {
      $rows[] = array(
        array('data' => bt('Scripts'), '#color' => 'bold_blue'),
        $info['scripts'] ? implode(', ', $info['scripts']) : 'none',
      );
    }

    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => ' ',
    );
    b_render($output);
  }
}
