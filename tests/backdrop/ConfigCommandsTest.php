<?php
/**
 * @file
 * PHPUnit tests for Bee Config commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Tests the Bee config commands.
 */
class ConfigCommandsTest extends TestCase {

  /**
   * Make sure that the config-get command works.
   */
  public function test_config_get_command_works() {
    $output_all = shell_exec('bee config-get system.core');
    $this->assertStringContainsString("'admin_theme' => 'seven',", (string) $output_all);
    $this->assertStringContainsString("'site_frontpage' => 'home',", (string) $output_all);

    $output_theme = shell_exec('bee config-get system.core theme_default');
    $this->assertStringContainsString("'basis'", (string) $output_theme);
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
    $this->assertStringContainsString((string) $value, (string) $output);

    // Set a new value for the homepage.
    $output = shell_exec("bee config-set $file $option $new");
    $this->assertStringContainsString("'$option' was set to '$new' in '$file'.", (string) $output);

    // Make sure the new homepage is 'about'.
    $output = shell_exec("bee config-get $file $option");
    $this->assertStringContainsString((string) $new, (string) $output);

    // Make sure 'empty' values can be set.
    $output = shell_exec("bee config-set $file $option2 $new2");
    $this->assertStringContainsString("'$option2' was set to '$new2' in '$file'.", (string) $output);

    // Reset config values for future tests.
    exec("bee config-set $file $option $value");
    exec("bee config-set $file $option2 $value2");
  }

  /**
   * Make sure that the config-clear command works.
   */
  public function test_config_clear_command_works() {
    $file = 'system.core';
    $option = 'foobar';
    $new = 'bazvalue';

    // Make sure nonexistent values can be set.
    $output = shell_exec("bee config-set $file $option $new");
    $this->assertStringContainsString("'$option' was set to '$new' in '$file'.", (string) $output);

    // Make sure values can be cleared.
    $output = shell_exec("bee config-clear $file $option");
    $this->assertStringContainsString("'$option' has been cleared from '$file'.", (string) $output);
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
    $this->assertStringContainsString('Config was exported to', (string) $output);

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
    $this->assertStringContainsString('Config was imported to', (string) $output);
    $this->assertStringContainsString('1 file was synced.', (string) $output);

    // Verify config file doesn't exist in active.
    $this->assertFileNotExists($file);

    // Put config file back.
    exec('mv dashboard.settings.json files/config_*/active/');
  }

}
