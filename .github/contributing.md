# Contributing

Contributions are welcome!

The following text assumes you're about to submit a new pull request.
For _how_ to contribute, please see [contributor documentation](https://dev.pioneerspacesim.net/contribute/).

### Check before submitting

- Have you read [code style](https://dev.pioneerspacesim.net/contribute/coding-conventions) in the development documentation?
- If new to github, make sure you're not opening this pull request from your _master_ branch, but rather a new [separate branch](https://dev.pioneerspacesim.net/contribute/git-and-github#making-a-pull-request)
- Have you reviewed your code? Don't trust the developers to do it, as they don't have time to read your code.
- Do you foresee any potential pitfalls or issues? If so please mention them.
- If a new feature, then please describe it, possibly with screenshot if something graphical.
- If a bug fix, then please use [key phrases](https://help.github.com/articles/closing-issues-via-commit-messages/) so that the original issue will be auto-closed upon merge. E.g.:

```
    fix #1234
    fixes #1234
    close #1234
    closes #1234
    resolve #1234
    resolves #1234
```


### Consider after submitting
After a pull request has been submitted, it is more common than not, that the branch keeps being modified, as bugs or issues are raised. If the change is to the latest commit on the branch, use `git add`, followed by `git commit --amend`, followed by a forced push, `git push -f`, to keep commit history clean. See our [documentation](https://dev.pioneerspacesim.net/contribute/git-and-github) for details on editing/fixing commits.

If you're hungry to contribute more, you'll find some pointers on [How you can contribute](https://wiki.pioneerspacesim.net/wiki/How_you_can_contribute).

Thanks for your contribution!
