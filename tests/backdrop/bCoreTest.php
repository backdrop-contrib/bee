<?php
/**
 * @file
 * PHPUnit tests for core Backdrop Console functionality.
 */

use PHPUnit\Framework\TestCase;

class bCoreTest extends TestCase {

  /**
   * Make sure that `b` is installed correctly.
   */
  public function test_b_is_installed_correctly() {
    // A '0' exit status means success.
    exec('b', $output, $result);
    $this->assertEquals(0, $result);
  }

  /**
   * Make sure that unknown commands trigger an error.
   */
  public function test_unknown_command_triggers_an_error() {
    $command = 'spoon';
    $output = shell_exec("b $command");
    $this->assertStringContainsString("There is no '$command' command.", $output);
  }

  /**
   * Make sure that required arguments are actually required.
   */
  public function test_missing_required_argument_triggers_an_error() {
    // `config-get` has a required argument: 'file'.
    $output = shell_exec('b config-get');
    $this->assertStringContainsString("Argument 'file' is required.", $output);
  }

  /**
   * Make sure that command aliases work.
   */
  public function test_commands_can_be_called_via_an_alias() {
    // `st` is an alias for `status`.
    $output_command = shell_exec('b status');
    $output_alias = shell_exec('b st');
    $this->assertEquals($output_command, $output_alias);
  }

  /**
   * Make sure that the `--root` option works.
   *
   * Test running `b` from outside the installation without the '--root' option,
   * and then with the '--root' option.
   */
  public function test_root_global_option_works() {
    $output_no_root = shell_exec('cd ../ && b status');
    $this->assertStringContainsString('No Backdrop installation found.', $output_no_root);

    $output_root = shell_exec('cd ../ && b --root=backdrop status');
    $this->assertStringNotContainsString('No Backdrop installation found.', $output_root);
  }

  /**
   * Make sure that the `--yes/--y` options work.
   */
  public function test_yes_global_option_works() {
    $file = 'backdrop.console';
    $option = 'foo';
    $value = 'bar';

    $output_yes = shell_exec("b config-set --yes $file $option $value");
    $this->assertStringContainsString("The '$file' config file doesn't exist.", $output_yes);
    $this->assertStringContainsString("'$option' was set to '$value'.", $output_yes);

    $file2 = $file . '2';

    $output_y = shell_exec("b config-set --y $file2 $option $value");
    $this->assertStringContainsString("The '$file2' config file doesn't exist.", $output_y);
    $this->assertStringContainsString("'$option' was set to '$value'.", $output_y);

    // Cleanup config files.
    // Using `find` allows us to find and delete the necessary files without
    // knowing the name of the config directory.
    exec("find files -type f -iname '$file.json' -delete");
    exec("find files -type f -iname '$file2.json' -delete");
  }

  /**
   * Make sure that the `--debug/--d` options work.
   */
  public function test_debug_global_option_works() {
    $output_debug = shell_exec("b status --debug");
    $this->assertStringContainsString("'Debug' mode enabled.", $output_debug);

    $output_d = shell_exec("b status --d");
    $this->assertStringContainsString("'Debug' mode enabled.", $output_d);
  }

}
