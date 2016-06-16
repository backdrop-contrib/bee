<?php
/**
 * Implementation of hook_b_command().
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
    'description' => 'Enable one or more extensions (modules or themes).',
    'arguments' => array(
      'extensions' => array(
        'description' => 'A list of modules or themes.',
        'multiple' => TRUE,
      ),
    ),
    'callback' => 'modules_b_enable_callback',
    'aliases' => array('en'),
  );
  $items['pm-disable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Disable one or more extensions (modules or themes).',
    'arguments' => array(
      'extensions' => array(
        'description' => 'A list of modules or themes.',
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
        'description' => 'A list of modules.',
        'multiple' => TRUE,
      ),
    ),
    'aliases' => array('pmu'),
    'callback' => 'modules_b_uninstall_callback',
  );
  return $items;
}

/**
 * pm-list callback. Display list of modules, themes and layouts.
 */
function modules_b_list_callback($arguments, $options){

  if(!isset($options['type']) or $options['type'] != 'layout') {
    $query = db_select('system', 's')
      ->fields('s');

    if(isset($options['type']) && in_array($options['type'], array('module', 'theme'))) {
      $query->condition('type', $options['type']);
    }

    $projects = $query->execute()->fetchAll();

    $rows = array();



    foreach($projects as $project) {
      $project->info = unserialize($project->info);
      if(isset($project->info['hidden']) && $project->info['hidden']){
        continue;
      }

      $status = array(
        'data' => '  [ ]  ',
        '#color' => 'bold_gray',
      );
      if($project->status){
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
        $project->info['version'],
      );

    }
  }

  if(!isset($options['type']) or $options['type'] == 'layout') {
    $used_layouts = layout_load_all();

    $layouts = layout_get_layout_template_info();
    foreach($layouts as $layout) {
      $name = array(
        'data' => $layout['name'],
        '#color' => 'bold_blue',
      );

      $status = FALSE;
      foreach($used_layouts as $used_layout){
        if($used_layout->layout_template == $layout['name']) {
          $status = array(
            'data' => '  [v]  ',
            '#color' => 'green',
          );
        }
      }
      if(!$status) {
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
 * pm-enable callback. Enable module or theme.
 */
function modules_b_enable_callback($arguments, $options) {
  if(!isset($arguments['extensions'])){
    b_set_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['extensions'] as $project ) {
    if(_enable_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to enable module or theme.
 */
function _enable_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $project = $query->execute()->fetchAssoc();

  if($project['status']) {
    b_set_message("Failed to enable module " . $project['name']. ": it is enabled already", 'error');
    return FALSE;
  }

  if($project['status'] == 0 && $project['type'] == 'module') {
    if(module_enable(array($project['name']), FALSE)) {
      b_set_message("Module " . $project['name'] . "  enabled", 'success');
      return TRUE;
    }
    else{
      b_set_message("Failed to enable module " . $project['name'], 'error');
    }
  }
  return FALSE;
}

/**
 * pm-disable callback. Disable module or theme.
 */
function modules_b_disable_callback($arguments, $options) {

  if(!isset($arguments['extensions'])){
    b_set_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['extensions'] as $project ) {
    if(_disable_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module or theme.
 */
function _disable_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $project = $query->execute()->fetchAssoc();

  if(!$project['status']) {
    b_set_message("Failed to disable module " . $project['name']. ": it is disabled already", 'error');
    return FALSE;
  }

  if($project['status'] == 1 && $project['type'] == 'module') {
    module_disable(array($project['name']), FALSE);
    b_set_message("Module " . $project['name'] . "  disabled", 'success');
    return TRUE;
  }
  return FALSE;
}

/**
 * pm-unstall callback. Uninstall module.
 */
function modules_b_uninstall_callback($arguments, $options) {
  if(!isset($arguments['modules'])){
    b_set_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['modules'] as $project ) {
    if(_uninstall_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module or theme.
 */
function _uninstall_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $project = $query->execute()->fetchAssoc();

  if($project['status']) {
    b_set_message("Failed to uninstall module " . $project['name']. ": it is need to be disabled first", 'error');
    return FALSE;
  }
  
  include_once BACKDROP_ROOT . '/core/includes/install.inc';

  if($project['status'] == 0 && $project['type'] == 'module') {
    if(backdrop_uninstall_modules(array($project['name']), FALSE)) {
      b_set_message("Module " . $project['name'] . "  uninstalled", 'success');
      return TRUE;
    }
    else{
      b_set_message("Failed to uninstall module " . $project['name'], 'error');
    }
  }
}
