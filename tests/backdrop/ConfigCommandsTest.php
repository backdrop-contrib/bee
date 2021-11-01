<?php
/**
 * @file
 * PHPUnit tests for Bee Config commands.
 */

use PHPUnit\Framework\TestCase;

class ConfigCommandsTest extends TestCase {

  /**
   * Make sure that the config-get command works.
   */
  public function test_config_get_command_works() {
    $output_all = shell_exec('bee config-get system.core');
    $this->assertStringContainsString("'admin_theme' => 'seven',", $output_all);
    $this->assertStringContainsString("'site_frontpage' => 'home',", $output_all);

    $output_theme = shell_exec('bee config-get system.core theme_default');
    $this->assertStringContainsString("'basis'", $output_theme);
  }

  /**
   * Make sure that the config-set command works.
   */
  public function test_config_set_command_works() {
    $file = 'system.core';
    $option = 'site_frontpage';
    $value = 'home';
    $new = 'about';
    $option2 = 'preprocess_css';
    $value2 = 1;
    $new2 = 0;

    // Make sure the current homepage is 'home'.
    $output = shell_exec("bee config-get $file $option");
    $this->assertStringContainsString($value, $output);

    // Set a new value for the homepage.
    $output = shell_exec("bee config-set $file $option $new");
    $this->assertStringContainsString("'$option' was set to '$new'.", $output);

    // Make sure the new homepage is 'about'.
    $output = shell_exec("bee config-get $file $option");
    $this->assertStringContainsString($new, $output);

    // Make sure 'empty' values can be set.
    $output = shell_exec("bee config-set $file $option2 $new2");
    $this->assertStringContainsString("'$option2' was set to '$new2'.", $output);

    // Reset config values for future tests.
    exec("bee config-set $file $option $value");
    exec("bee config-set $file $option2 $value2");
  }

  /**
   * Make sure that the config-export command works.
   */
  public function test_config_export_command_works() {
    // Number of files in active and staging should be different.
    exec('find files/config_*/active -type f', $active);
    exec('find files/config_*/staging -type f', $staging_before);
    $this->assertNotEquals(count($active), count($staging_before));

    $output = shell_exec('bee config-export');
    $this->assertStringContainsString('Config was exported to', $output);

    // Number of files in active and staging should be the same.
    exec('find files/config_*/staging -type f', $staging_after);
    $this->assertEquals(count($active), count($staging_after));
  }

  /**
   * Make sure that the config-import command works.
   */
  public function test_config_import_command_works() {
    // Remove config file from staging.
    exec('mv files/config_*/staging/dashboard.settings.json .');

    // Verify config file exists in active.
    $file = shell_exec("find files/config_*/active -name dashboard.settings.json | tr -d '\n'");
    $this->assertFileExists($file);

    $output = shell_exec('bee config-import -y');
    $this->assertStringContainsString('Config was imported to', $output);
    $this->assertStringContainsString('1 file was synced.', $output);

    // Verify config file doesn't exist in active.
    $this->assertFileNotExists($file);

    // Put config file back.
    exec('mv dashboard.settings.json files/config_*/active/');
  }

}
