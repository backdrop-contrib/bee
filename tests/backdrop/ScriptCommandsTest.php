<?php
/**
 * @file
 * PHPUnit tests for Bee PHP Script commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's PHP Script command.
 */
class ScriptCommandsTest extends TestCase {

  /**
   * Make sure that the php-script command works.
   */
  public function test_script_command_works() {
    // Create a simple script.
    exec('echo "<?php echo \"foo\" . \"bar\";" > /app/backdrop/simple.php');

    $this->assertTrue(file_exists('/app/backdrop/simple.php'));

    // Test simple command.
    $output_simple = shell_exec('bee scr simple.php');
    $this->assertStringContainsString('foobar', (string) $output_simple);

    // Remove for future tests.
    exec('rm /app/backdrop/simple.php');

    // Create a script that calls a Backdrop function.
    exec('echo "<?php config_set(\"test.settings\", \"value\", \"foobar\");" > /app/backdrop/backdrop-function.php');

    $this->assertTrue(file_exists('/app/backdrop/backdrop-function.php'));

    // Test that calling a Backdrop function works.
    exec('bee php-script /app/backdrop/backdrop-function.php');
    $output_backdrop = shell_exec('bee eval \'echo config_get("test.settings", "value")\'');
    $this->assertStringContainsString('foobar', (string) $output_backdrop);

    // Remove for future tests.
    exec('rm /app/backdrop/backdrop-function.php');

    // Cleanup test config file.
    exec('bee eval \'config("test.settings")->delete();\'');
  }

}
