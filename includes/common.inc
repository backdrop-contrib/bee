<?php
/*
 * @file
 * Common functions.
 */

/**
 * First bootstrap phase: initialize configuration.
 */
define('B_BOOTSTRAP_CONFIGURATION', 0);

/**
 * Second bootstrap phase: try to serve a cached page.
 */
define('B_BOOTSTRAP_PAGE_CACHE', 1);

/**
 * Third bootstrap phase: initialize database layer.
 */
define('B_BOOTSTRAP_DATABASE', 2);

/**
 * Fourth bootstrap phase: initialize the lock system.
 */
define('B_BOOTSTRAP_LOCK', 3);

/**
 * Fifth bootstrap phase: initialize the variable system.
 */
define('B_BOOTSTRAP_VARIABLES', 4);

/**
 * Sixth bootstrap phase: initialize session handling.
 */
define('B_BOOTSTRAP_SESSION', 5);

/**
 * Seventh bootstrap phase: set up the page header.
 */
define('B_BOOTSTRAP_PAGE_HEADER', 6);

/**
 * Eighth bootstrap phase: find out language of the page.
 */
define('B_BOOTSTRAP_LANGUAGE', 7);

/**
 * Final bootstrap phase: Backdrop is fully loaded; validate and fix input data.
 */
define('B_BOOTSTRAP_FULL', 8);

function b_print_error($text) {
  b_render(array(
    '#type' => 'text',
    '#color' => 'red',
    'value' => $text,
  )
  );
}
